/**
 * @file test_proposal_review_e2e.cpp
 * @brief END-TO-END integration test: proposal review with rejection,
 *        recovery, and live + final Visitor reporting.
 *
 * Exercises the full engine pipeline without mocking anything:
 *   1. Builder (Director + ConcreteWorkflowBuilder)   — builds definition
 *   2. Repository (InMemoryDefinitionRepository)      — stores definition
 *   3. Engine (WorkflowEngine::startWorkflow)         — starts instance
 *   4. Factory (InstanceFactory)                      — instantiates runtime tree
 *   5. Strategy (RoleBasedAssignment)                 — assigns participants
 *   6. State + Command                                — drives transitions
 *   7. Routing (SequentialRouting)                    — determines next step
 *   8. Observer (AuditObserver + EventBus)            — records all changes
 *   9. History (HistoryManager)                       — records all commands
 *  10. Visitor (ProgressReportVisitor, WorkloadVisitor) — reports on live tree
 *
 * Business scenario (mirrors main.cpp):
 *   1. Deon submits a proposal
 *   2. Lerato (Product Manager) reviews — rejects for unclear scope
 *   3. Lerato tries to complete the rejected review — State pattern blocks it
 *   4. Proposal is re-offered, re-assigned, and successfully completed
 *   5. Thandi (Head Engineer) does the technical review
 *   6. ProgressVisitor runs mid-workflow (partial tree) and at the end
 *   7. WorkloadVisitor summarizes who did what
 */
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "ConcreteWorkflowBuilder.h"
#include "WorkflowDefinition.h"
#include "InMemoryDefinitionRepository.h"
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"
#include "WorkItem.h"
#include "Participant.h"
#include "RoleBasedAssignment.h"
#include "SequentialRouting.h"
#include "RoutingHelper.h"
#include "EventBus.h"
#include "AuditObserver.h"
#include "HistoryManager.h"
#include "AssignCommand.h"
#include "StartCommand.h"
#include "CompleteCommand.h"
#include "makeAvailableCommand.h"
#include "RejectCommand.h"
#include "ProgressReportVisitor.h"
#include "WorkloadVisitor.h"

namespace {

/** @brief Builds the standard proposal-approval blueprint. */
WorkflowBlueprint makeProposalBlueprint() {
    WorkflowBlueprint bp;
    bp.id      = "proposal-approval";
    bp.name    = "Proposal Approval Workflow";
    bp.version = 1;

    // Roles
    bp.roles.push_back(RoleDefinition("proposer", "Proposer", 0));
    bp.roles.push_back(RoleDefinition("product_manager", "Product Manager", 1));
    bp.roles.push_back(RoleDefinition("head_engineer", "Head Engineer", 2));

    // Flat activities
    bp.activities.push_back(ActivityDefinition("submit", "Submit Proposal",  "proposer"));
    bp.activities.push_back(ActivityDefinition("review", "Product Review",   "product_manager"));
    bp.activities.push_back(ActivityDefinition("tech",   "Technical Review", "head_engineer"));

    // Transitions
    bp.transitions.push_back(TransitionDefinition("submit", "review", "always"));
    bp.transitions.push_back(TransitionDefinition("review", "tech",   "always"));

    // Composite tree
    WorkflowNodeData root;
    root.type = NodeType::STAGE;
    root.id   = "root";
    root.name = "Proposal Process";

    WorkflowNodeData submitNode;
    submitNode.type   = NodeType::ACTIVITY;
    submitNode.id     = "submit";
    submitNode.name   = "Submit Proposal";
    submitNode.roleId = "proposer";
    root.children.push_back(submitNode);

    WorkflowNodeData reviewNode;
    reviewNode.type   = NodeType::ACTIVITY;
    reviewNode.id     = "review";
    reviewNode.name   = "Product Review";
    reviewNode.roleId = "product_manager";
    root.children.push_back(reviewNode);

    WorkflowNodeData techNode;
    techNode.type   = NodeType::ACTIVITY;
    techNode.id     = "tech";
    techNode.name   = "Technical Review";
    techNode.roleId = "head_engineer";
    root.children.push_back(techNode);

    bp.rootTreeData = root;
    bp.hasRootTree  = true;
    return bp;
}

/**
 * @brief Drives a work item to Completed via the engine's command interface.
 * Handles whatever initial state the factory put the item in.
 */
void driveToCompleted(WorkflowEngine& engine, WorkItem* item, Participant* actor) {
    if (item->getState() == "Created")     makeAvailableCommand(item).execute();
    if (item->getState() == "Available")   engine.submit(new AssignCommand(item, actor));
    if (item->getState() == "Assigned")    engine.submit(new StartCommand(item));
    if (item->getState() == "In_Progress") engine.submit(new CompleteCommand(item));
}

} // namespace

// END-TO-END: Proposal review with rejection, recovery, and Visitor reporting
TEST(ProposalReviewE2E, FullPipelineWithRejectionRecoveryAndVisitors) {

    // STAGE 1: BUILD (Director + Builder)
    WorkflowDirector director;
    ConcreteWorkflowBuilder builder;
    WorkflowBlueprint bp = makeProposalBlueprint();
    director.construct(builder, bp);
    WorkflowDefinition* definition = new WorkflowDefinition(builder.build());

    ASSERT_NE(definition->getRoot(), nullptr) << "Builder produced no tree";
    EXPECT_EQ(definition->getActivities().size(), 3u);
    EXPECT_EQ(definition->getTransitions().size(), 2u);

    // STAGE 2: SAVE (Repository)
    InMemoryDefinitionRepository repo;
    repo.save(definition);
    EXPECT_EQ(repo.findAll().size(), 1u);

    // STAGE 3: SETUP (Observer + History + Engine)
    EventBus bus;
    AuditObserver audit;
    bus.attach(&audit);
    HistoryManager history;
    WorkflowEngine engine(&repo, &bus, &history, &audit);
    
    // STAGE 4: PARTICIPANTS + STRATEGY
    Participant deon("p1", "Deon", {"proposer"});
    Participant lerato("p2", "Lerato", {"product_manager"});
    Participant thandi("p3", "Thandi", {"head_engineer"});
    std::vector<Participant*> staff = {&deon, &lerato, &thandi};

    RoleBasedAssignment assignment;
    engine.setAssignment(&assignment, staff);

    // STAGE 5: START (Engine + Factory)
    std::string instanceId = engine.startWorkflow("proposal-approval", 1);
    ASSERT_FALSE(instanceId.empty()) << "Engine failed to start workflow";

    WorkflowInstance* instance = engine.getInstance(instanceId);
    ASSERT_NE(instance, nullptr);
    ASSERT_NE(instance->getRoot(), nullptr) << "Factory produced no runtime tree";
    EXPECT_EQ(instance->getState(), "RUNNING");

    // Factory should have assigned all three items via the strategy.
    WorkItem* submit = instance->findWorkItem("submit");
    WorkItem* review = instance->findWorkItem("review");
    WorkItem* tech   = instance->findWorkItem("tech");
    ASSERT_NE(submit, nullptr);
    ASSERT_NE(review, nullptr);
    ASSERT_NE(tech,   nullptr);

    EXPECT_EQ(submit->getAssignedParticipant(), &deon);
    EXPECT_EQ(review->getAssignedParticipant(), &lerato);
    EXPECT_EQ(tech->getAssignedParticipant(),   &thandi);

    // STAGE 6: DRIVE THE WORKFLOW (Command + State + Routing)
    SequentialRouting sequential;
    const ActivityDefinition* submitDef = findActivity(*definition, "submit");
    const ActivityDefinition* reviewDef = findActivity(*definition, "review");

    // ----- Step 1: submit (happy path) -----
    driveToCompleted(engine, submit, &deon);
    ASSERT_EQ(submit->getState(), "Completed");

    // ----- Visitor mid-check: progress on a partially-complete tree -----
    // The Visitor pattern is most useful while the workflow is still running,
    // not just at the end. Verify it can walk a half-complete tree correctly.
    {
        ProgressReportVisitor midReport;
        ASSERT_TRUE(engine.applyVisitor(instanceId, midReport));

        EXPECT_EQ(midReport.getTotalItems(),     3);
        EXPECT_EQ(midReport.getCompletedItems(), 1);   // only "submit" is done
        EXPECT_FALSE(midReport.isComplete());
        EXPECT_LT(midReport.getPercentComplete(), 100);

        // The report should mention the completed item
        std::vector<std::string> lines = midReport.getReport();
        bool foundSubmit = false;
        for (const std::string& line : lines) {
            if (line.find("Submit Proposal") != std::string::npos) {
                foundSubmit = true;
                break;
            }
        }
        EXPECT_TRUE(foundSubmit) << "Visitor report should mention the completed item";
    }

    auto nextAfterSubmit = sequential.next(*submitDef, *definition, "approved");
    ASSERT_EQ(nextAfterSubmit.size(), 1u);
    EXPECT_EQ(nextAfterSubmit[0]->getId(), "review");

    // ----- Step 2: productReview (REJECTION SCENARIO) -----
    // Lerato rejects because scope is unclear
    EXPECT_TRUE(engine.submit(new RejectCommand(review)));
    ASSERT_EQ(review->getState(), "Rejected");

    // Lerato tries to complete the rejected review — must be refused by State pattern
    bool illegalComplete = engine.submit(new CompleteCommand(review));
    EXPECT_FALSE(illegalComplete) << "State pattern should block completion from Rejected";
    EXPECT_EQ(review->getState(), "Rejected") << "State must not change after refused command";

    // Proposal is re-offered (Deon clarifies scope)
    makeAvailableCommand(review).execute();
    if (review->getState() == "Available") {
        driveToCompleted(engine, review, &lerato);
    }
    ASSERT_EQ(review->getState(), "Completed") << "Review should complete after re-offer";

    auto nextAfterReview = sequential.next(*reviewDef, *definition, "approved");
    ASSERT_EQ(nextAfterReview.size(), 1u);
    EXPECT_EQ(nextAfterReview[0]->getId(), "tech");

    // ----- Step 3: tech (happy path) -----
    driveToCompleted(engine, tech, &thandi);
    ASSERT_EQ(tech->getState(), "Completed");

    // STAGE 7: VERIFY OBSERVER CAPTURED EVERYTHING
    const auto& auditLog = audit.getLog();
    EXPECT_GT(auditLog.size(), 0u);

    bool foundRejection  = false;
    bool foundRecovery   = false;
    bool foundCompletion = false;
    for (const std::string& line : auditLog) {
        if (line.find("-> Rejected")  != std::string::npos) foundRejection  = true;
        if (line.find("-> Available") != std::string::npos) foundRecovery   = true;
        if (line.find("-> Completed") != std::string::npos) foundCompletion = true;
    }
    EXPECT_TRUE(foundRejection)  << "Observer missed the rejection event";
    EXPECT_TRUE(foundRecovery)   << "Observer missed the recovery event";
    EXPECT_TRUE(foundCompletion) << "Observer missed completion events";

    // STAGE 8: VERIFY HISTORY
    EXPECT_GT(history.getAll().size(), 0u);

    // STAGE 9: VERIFY FINAL TREE STATE
    EXPECT_EQ(submit->getState(), "Completed");
    EXPECT_EQ(review->getState(), "Completed");
    EXPECT_EQ(tech->getState(),   "Completed");

    // Instance remains RUNNING — the engine does not auto-complete it
    EXPECT_EQ(instance->getState(), "RUNNING");

    // STAGE 10: COMPLETION REPORT (ProgressReportVisitor)
    ProgressReportVisitor finalReport;
    ASSERT_TRUE(engine.applyVisitor(instanceId, finalReport));

    EXPECT_EQ(finalReport.getTotalItems(),     3);
    EXPECT_EQ(finalReport.getCompletedItems(), 3);
    EXPECT_EQ(finalReport.getPercentComplete(), 100);
    EXPECT_TRUE(finalReport.isComplete())
        << "ProgressReportVisitor should report the workflow as complete";

    // The final report line should be "3/3 work items completed (100%)"
    std::vector<std::string> report = finalReport.getReport();
    ASSERT_GE(report.size(), 1u);
    const std::string& summary = report.back();
    EXPECT_NE(summary.find("3/3"),  std::string::npos) << "Summary should show 3/3 items";
    EXPECT_NE(summary.find("100%"), std::string::npos) << "Summary should show 100%";

    // State counts should reflect the three completed items
    auto counts = finalReport.getStateCounts();
    EXPECT_EQ(counts["Completed"], 3);

    // STAGE 11: WORKLOAD SUMMARY (WorkloadVisitor)
    WorkloadVisitor workload;
    ASSERT_TRUE(engine.applyVisitor(instanceId, workload));

    // Each participant should have exactly one item, all completed (zero open)
    auto deonItems   = workload.getItemsFor("Deon");
    auto leratoItems = workload.getItemsFor("Lerato");
    auto thandiItems = workload.getItemsFor("Thandi");

    ASSERT_EQ(deonItems.size(),   1u);
    ASSERT_EQ(leratoItems.size(), 1u);
    ASSERT_EQ(thandiItems.size(), 1u);

    EXPECT_EQ(deonItems[0],   "Submit Proposal");
    EXPECT_EQ(leratoItems[0], "Product Review");
    EXPECT_EQ(thandiItems[0], "Technical Review");

    // All items are Completed, so openCount should be 0 for everyone
    EXPECT_EQ(workload.getOpenCount("Deon"),   0);
    EXPECT_EQ(workload.getOpenCount("Lerato"), 0);
    EXPECT_EQ(workload.getOpenCount("Thandi"), 0);

    // Nothing should be unassigned (the strategy assigned everything)
    EXPECT_TRUE(workload.getUnassigned().empty());

    // The summary should have one line per participant
    std::vector<std::string> wlSummary = workload.getSummary();
    EXPECT_EQ(wlSummary.size(), 3u);

    // Each summary line should mention its participant's name
    bool foundDeon   = false;
    bool foundLerato = false;
    bool foundThandi = false;
    for (const std::string& line : wlSummary) {
        if (line.find("Deon")   != std::string::npos) foundDeon   = true;
        if (line.find("Lerato") != std::string::npos) foundLerato = true;
        if (line.find("Thandi") != std::string::npos) foundThandi = true;
    }
    EXPECT_TRUE(foundDeon)   << "Workload summary should mention Deon";
    EXPECT_TRUE(foundLerato) << "Workload summary should mention Lerato";
    EXPECT_TRUE(foundThandi) << "Workload summary should mention Thandi";
}