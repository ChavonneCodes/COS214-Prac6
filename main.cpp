/**
 * @file main.cpp
 * @brief TIVIDY demo: one project proposal at a tech company, run through the whole system.
 *
 * A developer submits a project proposal. It is reviewed by the product
 * manager, then by a head software engineer, then finance checks the budget.
 * Budgets above the executive limit also need executive sign-off before the
 * project can kick off.
 *
 * Patterns shown, in the order they appear:
 * - Facade: WorkflowEngine is the only entry point for starting work and submitting commands;
 * - Observer: every state change is published on the EventBus to the audit log and notifications;
 * - Builder + Composite: the Director and Builder build the definition tree (two versions);
 * - Factory Method: InstanceFactory turns the definition tree into a runtime tree;
 * - Strategy: WorkloadBasedAssignment picks who does each work item,
 *   Sequential/ConditionalRouting picks what happens next;
 * - Decorator: the runtime tree is labelled with its instance id and pinned version;
 * - Command + State: every action is a command, and the work item's state object decides if it is allowed;
 * - Chain of Responsibility: finance and executive approvals, plus timeout escalation;
 * - Memento + Prototype: a work item's state is cloned into a memento and restored after a mistake;
 * - Visitor: progress after every step, then a completion report and a workload summary,
 *   run over the runtime tree through the engine.
 */

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "ConcreteWorkflowBuilder.h"
#include "InMemoryDefinitionRepository.h"
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"
#include "EventBus.h"
#include "AuditObserver.h"
#include "NotificationObserver.h"
#include "HistoryManager.h"
#include "WorkItem.h"
#include "WorkItemMemento.h"
#include "Data.h"
#include "Participant.h"
#include "WorkloadBasedAssignment.h"
#include "SequentialRouting.h"
#include "ConditionalRouting.h"
#include "RoutingHelper.h"
#include "Request.h"
#include "RoleApprovalHandler.h"
#include "TimeoutEscalationHandler.h"
#include "AssignCommand.h"
#include "StartCommand.h"
#include "CompleteCommand.h"
#include "RejectCommand.h"
#include "EscalateCommand.h"
#include "ProgressReportVisitor.h"
#include "WorkloadVisitor.h"

namespace {

const std::string DEFINITION_ID = "project-approval";
const int EXECUTIVE_LIMIT = 500000;   ///< Budgets above this (in rand) need executive sign-off.
const int PROPOSAL_BUDGET = 750000;   ///< Budget of the proposal in this run.

/**
 * @brief Prints a section heading.
 * @param title Text of the heading.
 */
void heading(const std::string& title) {
    std::cout << "\n=================================================================\n"
              << " " << title << "\n"
              << "=================================================================\n";
}

/**
 * @brief Makes the data for one activity node of the definition tree.
 * @param id     Activity id.
 * @param name   Display name.
 * @param roleId Role needed to perform it.
 * @return The node data.
 */
WorkflowNodeData activityNode(const std::string& id, const std::string& name, const std::string& roleId) {
    WorkflowNodeData node;
    node.type = NodeType::ACTIVITY;
    node.id = id;
    node.name = name;
    node.roleId = roleId;
    return node;
}

/**
 * @brief Makes the data for one stage node of the definition tree.
 * @param id   Stage id.
 * @param name Display name.
 * @return The node data, with no children yet.
 */
WorkflowNodeData stageNode(const std::string& id, const std::string& name) {
    WorkflowNodeData node;
    node.type = NodeType::STAGE;
    node.id = id;
    node.name = name;
    return node;
}

/**
 * @brief Builds the blueprint for the "Project Approval" workflow.
 *
 * The blueprint holds the Composite tree (stages and activities), the flat
 * activity list and the transitions that SequentialRouting follows.
 * Version 2 adds a security review after the technical review.
 * @param version 1 or 2.
 * @return The filled-in blueprint.
 */
WorkflowBlueprint makeProjectApprovalBlueprint(int version) {
    bool withSecurityReview = version >= 2;

    WorkflowBlueprint blueprint;
    blueprint.id = DEFINITION_ID;
    blueprint.name = "Tech Company Project Approval";
    blueprint.version = version;

    blueprint.roles.push_back(RoleDefinition("proposer", "Proposer", 0));
    blueprint.roles.push_back(RoleDefinition("product_manager", "Product Manager", 1));
    blueprint.roles.push_back(RoleDefinition("head_engineer", "Head Software Engineer", 2));
    blueprint.roles.push_back(RoleDefinition("finance", "Finance Officer", 2));
    blueprint.roles.push_back(RoleDefinition("executive", "Executive", 3));

    blueprint.activities.push_back(ActivityDefinition("submit", "Submit Proposal", "proposer"));
    blueprint.activities.push_back(ActivityDefinition("productReview", "Product Review", "product_manager"));
    blueprint.activities.push_back(ActivityDefinition("techReview", "Technical Review", "head_engineer"));
    if (withSecurityReview) {
        blueprint.activities.push_back(ActivityDefinition("securityReview", "Security Review", "head_engineer"));
    }
    blueprint.activities.push_back(ActivityDefinition("financeReview", "Budget Review", "finance"));
    blueprint.activities.push_back(ActivityDefinition("execApproval", "Executive Approval", "executive"));
    blueprint.activities.push_back(ActivityDefinition("kickoff", "Project Kickoff", "proposer"));

    // financeReview has no transition here: ConditionalRouting decides where it goes.
    blueprint.transitions.push_back(TransitionDefinition("submit", "productReview", "always"));
    blueprint.transitions.push_back(TransitionDefinition("productReview", "techReview", "always"));
    if (withSecurityReview) {
        blueprint.transitions.push_back(TransitionDefinition("techReview", "securityReview", "always"));
        blueprint.transitions.push_back(TransitionDefinition("securityReview", "financeReview", "always"));
    } else {
        blueprint.transitions.push_back(TransitionDefinition("techReview", "financeReview", "always"));
    }
    blueprint.transitions.push_back(TransitionDefinition("execApproval", "kickoff", "always"));

    // Composite tree: Project Approval -> [Submit, Review stage, Budget stage, Kickoff]
    WorkflowNodeData root = stageNode("root", "Project Approval");
    root.children.push_back(activityNode("submit", "Submit Proposal", "proposer"));

    WorkflowNodeData review = stageNode("reviewStage", "Review Stage");
    review.children.push_back(activityNode("productReview", "Product Review", "product_manager"));
    review.children.push_back(activityNode("techReview", "Technical Review", "head_engineer"));
    if (withSecurityReview) {
        review.children.push_back(activityNode("securityReview", "Security Review", "head_engineer"));
    }
    root.children.push_back(review);

    WorkflowNodeData budget = stageNode("budgetStage", "Budget Stage");
    budget.children.push_back(activityNode("financeReview", "Budget Review", "finance"));
    budget.children.push_back(activityNode("execApproval", "Executive Approval", "executive"));
    root.children.push_back(budget);

    root.children.push_back(activityNode("kickoff", "Project Kickoff", "proposer"));

    blueprint.rootTreeData = root;
    blueprint.hasRootTree = true;
    return blueprint;
}

/**
 * @brief Submits a command through the engine and prints the state change.
 * @param engine  The engine (takes ownership of the command).
 * @param command Command to run.
 * @param item    Work item the command acts on.
 * @return true if the work item's state allowed the action.
 */
bool run(WorkflowEngine& engine, WorkflowAction* command, WorkItem* item) {
    std::string before = item->getCurrentState()->getName();
    std::cout << "  > " << command->getDescription() << "\n";
    bool ok = engine.submit(command);
    std::cout << "    state: " << before << " -> " << item->getCurrentState()->getName()
              << (ok ? "" : "  (not allowed, nothing recorded)") << "\n";
    return ok;
}

/**
 * @brief Runs the normal path for a work item: assign, start, complete.
 * @param engine The engine.
 * @param item   Work item to work on.
 */
void assignStartComplete(WorkflowEngine& engine, WorkItem* item) {
    run(engine, new AssignCommand(item, item->getAssignedParticipant()), item);
    run(engine, new StartCommand(item), item);
    run(engine, new CompleteCommand(item), item);
}

} // namespace

int main() {
    heading("TIVIDY DEMO: TECH COMPANY PROJECT APPROVAL");
    std::cout << "Proposal budget: R" << PROPOSAL_BUDGET
              << " (executive sign-off needed above R" << EXECUTIVE_LIMIT << ")\n";

    // ---------------------------------------------------------------
    heading("1. SET UP THE ENGINE (Facade) AND ITS SUBSYSTEMS (Observer)");
    InMemoryDefinitionRepository repository;
    EventBus eventBus;
    HistoryManager history;
    AuditObserver audit;
    NotificationObserver decisions("APPROVAL_DECISION");
    eventBus.attach(&audit);
    eventBus.attach(&decisions);
    WorkflowEngine engine(&repository, &eventBus, &history, &audit);
    std::cout << "Engine ready with " << eventBus.observerCount() << " observers on the event bus.\n";

    // ---------------------------------------------------------------
    heading("2. BUILD AND STORE THE DEFINITION (Builder + Composite)");
    WorkflowDirector director;
    for (int version = 1; version <= 2; ++version) {
        ConcreteWorkflowBuilder builder;
        WorkflowBlueprint blueprint = makeProjectApprovalBlueprint(version);
        director.construct(builder, blueprint);
        repository.save(new WorkflowDefinition(builder.build()));
    }

    WorkflowDefinition* latest = repository.loadLatest(DEFINITION_ID);
    if (!latest || !latest->getRoot()) {
        std::cerr << "ERROR: the definition was not stored correctly.\n";
        return 1;
    }
    std::cout << "The repository holds " << repository.findAll().size() << " versions of '"
              << latest->getName() << "'. Latest is v" << latest->getVersion() << ":\n";
    latest->getRoot()->print(1);

    // ---------------------------------------------------------------
    heading("3. START A WORKFLOW INSTANCE (Factory Method + Strategy + Decorator)");
    Participant deon("p1", "Deon", {"proposer"}, 0);
    Participant lerato("p2", "Lerato", {"product_manager"}, 1);
    Participant sipho("p3", "Sipho", {"head_engineer"}, 2);
    Participant thandi("p4", "Thandi", {"head_engineer"}, 2);
    Participant ayesha("p5", "Ayesha", {"finance"}, 2);
    Participant naledi("p6", "Naledi", {"executive"}, 3);
    Participant mandla("p7", "Mandla", {"executive"}, 4);
    sipho.addOpenItem();   // Sipho is already busy, so the workload
    sipho.addOpenItem();   // strategy should pick Thandi for the tech review.
    std::vector<Participant*> staff = {&deon, &lerato, &sipho, &thandi, &ayesha, &naledi, &mandla};

    WorkloadBasedAssignment assignment;
    engine.setAssignment(&assignment, staff);

    std::cout << "Trying to start a definition that does not exist...\n";
    std::string missing = engine.startWorkflow("does-not-exist", 1);
    std::cout << "  startWorkflow returned " << (missing.empty() ? "no instance (start error reported)" : missing) << "\n";

    std::cout << "\nStarting '" << DEFINITION_ID << "' pinned to v1 (assignment strategy: "
              << assignment.getName() << ")\n";
    std::string instanceId = engine.startWorkflow(DEFINITION_ID, 1);
    WorkflowInstance* instance = engine.getInstance(instanceId);
    if (!instance) {
        std::cerr << "ERROR: the engine could not start the workflow.\n";
        return 1;
    }
    instance->getData().setValue(std::to_string(PROPOSAL_BUDGET));
    WorkflowDefinition* definition = repository.load(DEFINITION_ID, instance->getDefinitionVersion());

    std::cout << "\nInstance " << instanceId << " is " << instance->getState() << " on v"
              << instance->getDefinitionVersion() << " (v" << latest->getVersion()
              << " has a security review, this instance does not). Runtime tree:\n";
    instance->getRoot()->print(1);

    // ---------------------------------------------------------------
    // Chain of Responsibility: finance -> executive -> timeout escalation
    RoleApprovalHandler financeApprover("finance");
    RoleApprovalHandler executiveApprover("executive");
    TimeoutEscalationHandler timeoutEscalation;
    financeApprover.setNext(&executiveApprover);
    executiveApprover.setNext(&timeoutEscalation);

    // Routing strategies: sequential by default, budget check after finance review
    SequentialRouting sequential;
    ConditionalRouting budgetCheck("over_limit", "execApproval", "kickoff");
    std::map<std::string, RoutingStrategy*> routingFor;
    routingFor["financeReview"] = &budgetCheck;

    // What happens inside each activity. Each returns the outcome used for routing.
    std::map<std::string, std::function<std::string(WorkItem*)>> behaviour;

    behaviour["productReview"] = [&](WorkItem* item) {
        std::cout << "  Lerato sends the proposal back: the scope is unclear.\n";
        run(engine, new AssignCommand(item, item->getAssignedParticipant()), item);
        run(engine, new RejectCommand(item), item);
        std::cout << "  Lerato tries to complete the rejected review anyway...\n";
        run(engine, new CompleteCommand(item), item);
        std::cout << "  Deon clarifies the scope and the review is re-offered.\n";
        item->makeAvailable();
        assignStartComplete(engine, item);
        return std::string("approved");
    };

    behaviour["techReview"] = [&](WorkItem* item) {
        run(engine, new AssignCommand(item, item->getAssignedParticipant()), item);
        run(engine, new StartCommand(item), item);

        // Memento + Prototype: snapshot a clone of the current state before anything risky
        Data notes("Architecture review: microservices design accepted");
        WorkItemMemento snapshot("techReview", item->getId(), notes, item->getCurrentState()); //changed item->getCurrentState()->clone() -> item->getCurrentState() 
        std::cout << "  Saved memento of '" << snapshot.getWorkItemId() << "' in state "
                  << snapshot.getStateKind()->getName() << "\n";

        std::string wrongRole = "executive";
        std::cout << "  Thandi escalates the review by mistake.\n";
        run(engine, new EscalateCommand(item, wrongRole), item);

        std::cout << "  Rolling back: undo the last command and restore the memento.\n";
        engine.undoLast();
        item->changeState(snapshot.getStateKind()->clone()); // the work item now owns the saved state
        std::cout << "    state restored to " << item->getCurrentState()->getName()
                  << ", notes: \"" << snapshot.getDataSnapshot().getValue() << "\"\n";

        run(engine, new CompleteCommand(item), item);
        return std::string("approved");
    };

    behaviour["financeReview"] = [&](WorkItem* item) {
        run(engine, new AssignCommand(item, item->getAssignedParticipant()), item);
        run(engine, new StartCommand(item), item);

        std::cout << "  Asking the approval chain to sign off the budget...\n";
        Request request(
            "REQ-FIN-001",
            "APPROVE",
            item->getId(),
            instanceId,
            "finance",
            "finance-reviewer",
            "Budget approval",
            item->getCurrentState()->getName(),
            instance->getData()
        );

        HandlerResult financeResult = financeApprover.authorise(request);

        if (financeResult == HandlerResult::Rejected) {
            std::cout << "Finance approval was rejected.\n";
        }

        financeApprover.authorise(request);

        std::string outcome = std::stoi(instance->getData().getValue()) > EXECUTIVE_LIMIT
                                  ? "over_limit" : "within_limit";
        eventBus.notify(WorkFlowEvent("APPROVAL_DECISION", instanceId, item->getId(),
                                      "budget approved by finance (" + outcome + ")"));
        run(engine, new CompleteCommand(item), item);
        return outcome;
    };

    behaviour["execApproval"] = [&](WorkItem* item) {
        run(engine, new AssignCommand(item, item->getAssignedParticipant()), item);
        std::cout << "  The approval deadline passes without a decision from Naledi...\n";
        Request timeout(
            "REQ-TIMEOUT-001",
            "TIMEOUT",
            item->getId(),
            instanceId,
            "board",
            "system",
            "Executive approval deadline exceeded",
            item->getCurrentState()->getName(),
            instance->getData()
        );

        HandlerResult timeoutResult = financeApprover.authorise(timeout);

        if (timeoutResult == HandlerResult::Handled) {
            std::string target = "executive";
            run(engine, new EscalateCommand(item, target), item);
        }

        std::cout << "  Mandla (CEO) picks up the escalated approval.\n";
        run(engine, new AssignCommand(item, &mandla), item);
        Request signOff(
            "REQ-EXEC-001",
            "APPROVE",
            item->getId(),
            instanceId,
            "executive",
            "executive-reviewer",
            "Executive project sign-off",
            item->getCurrentState()->getName(),
            instance->getData()
        );

        HandlerResult signOffResult = executiveApprover.authorise(signOff);

        if (signOffResult == HandlerResult::Rejected) {
            std::cout << "Executive approval was rejected.\n";
        }

        eventBus.notify(WorkFlowEvent("APPROVAL_DECISION", instanceId, item->getId(),
                                      "project approved by the executive"));
        run(engine, new StartCommand(item), item);
        run(engine, new CompleteCommand(item), item);
        return std::string("approved");
    };

    // ---------------------------------------------------------------
    heading("4. RUN THE WORKFLOW (Command + State + Strategy + Chain + Memento + Visitor)");
    std::string currentId = "submit";
    while (!currentId.empty()) {
        WorkItem* item = instance->findWorkItem(currentId);
        Participant* who = item->getAssignedParticipant();
        std::cout << "\n--- " << currentId << " (" << (who ? who->getName() : "unassigned") << ") ---\n";

        item->makeAvailable();
        std::string outcome = "approved";
        if (behaviour.count(currentId)) {
            outcome = behaviour[currentId](item);
        } else {
            assignStartComplete(engine, item);
        }

        const ActivityDefinition* activity = findActivity(*definition, currentId);
        RoutingStrategy* routing = routingFor.count(currentId) ? routingFor[currentId] : &sequential;
        std::vector<const WorkflowComponent*> next = routing->next(*activity, *definition, outcome);

        currentId = next.empty() ? "" : next[0]->getId();
        std::cout << "  Routing (" << routing->getName() << ", outcome '" << outcome << "'): "
                  << (currentId.empty() ? "end of workflow" : "next is " + currentId) << "\n";

        // Visitor: a fresh progress visitor walks the whole tree after every step
        ProgressReportVisitor progress;
        engine.applyVisitor(instanceId, progress);
        std::cout << "  Progress: " << progress.getCompletedItems() << "/" << progress.getTotalItems()
                  << " work items completed (" << progress.getPercentComplete() << "%)\n";
    }
    instance->setState("COMPLETED");

    // ---------------------------------------------------------------
    heading("5. RESULTS (all read back through the Facade)");
    std::cout << "Instance " << instanceId << " is " << engine.getInstance(instanceId)->getState()
              << ". Final runtime tree:\n";
    engine.getInstance(instanceId)->getRoot()->print(1);

    std::vector<std::string> trail = engine.getAuditTrail(instanceId);
    std::cout << "\nAudit trail (" << trail.size() << " entries):\n";
    for (const std::string& line : trail) std::cout << "  " << line << "\n";

    std::cout << "\nNotifications sent to the proposer:\n";
    for (const std::string& line : decisions.getNotifications()) std::cout << "  " << line << "\n";

    std::vector<WorkflowAction*> done = engine.getHistory(instanceId);
    std::cout << "\nCommand history (" << done.size() << " commands that changed state):\n";
    for (WorkflowAction* action : done) std::cout << "  " << action->getDescription() << "\n";

    // ---------------------------------------------------------------
    heading("6. REPORTS (Visitor)");
    std::cout << "Two new operations over the runtime tree, added without changing WorkItem,\n"
              << "StageInstance or SubProcessInstance. The engine passes each visitor to the tree.\n";

    ProgressReportVisitor report;
    engine.applyVisitor(instanceId, report);
    std::cout << "\nCompletion report for " << instanceId << ":\n";
    for (const std::string& line : report.getReport()) std::cout << "  " << line << "\n";
    std::cout << "  Workflow complete: " << (report.isComplete() ? "yes" : "no") << "\n";

    WorkloadVisitor workload;
    engine.applyVisitor(instanceId, workload);
    std::cout << "\nWorkload by participant:\n";
    for (const std::string& line : workload.getSummary()) std::cout << "  " << line << "\n";

    heading("DEMO COMPLETE");
    return 0;
}
