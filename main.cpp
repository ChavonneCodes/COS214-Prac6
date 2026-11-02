/**
 * @file main.cpp
 * @brief TIVIDY demo: one engine runs two unrelated processes.
 *
 * Scenario 1 is a campus portal outage. Scenario 2 is a project proposal.
 * This file holds only what differs between them. Everything they share is in
 * DemoRunner.h. The console prints a short overview: the demo itself is
 * gui/index.html, which replays the run saved to gui/trace.js.
 * Run with --verbose to print every step as well.
 */

#include <sstream>

#include "AllowedStatesRuntimeDecorator.h"
#include "AssignCommand.h"
#include "CancelCommand.h"
#include "ConditionalRouting.h"
#include "DemoRunner.h"
#include "LoggingRuntimeDecorator.h"
#include "makeAvailableCommand.h"
#include "Participant.h"
#include "RejectCommand.h"
#include "Request.h"
#include "SecurityRuntimeDecorator.h"
#include "ValidationApprovalHandler.h"
#include "WorkloadBasedAssignment.h"

namespace {

const int EXECUTIVE_LIMIT = 500000; ///< Budgets above this (in rand) need executive sign-off.

/** @brief Scenario 1: an outage, a flat process, and a repair that has to be repeated. */
void runCampusRecovery(Demo& demo, Participant& bob) {
    ValidationApprovalHandler validation;
    RoleApprovalHandler technicianApproval("technician"), administratorApproval("administrator");
    validation.setNext(&technicianApproval);
    technicianApproval.setNext(&administratorApproval);
    ConditionalRouting verification("passed", "close", "repair");

    Participant student("c0", "Student", {"guest"});
    SecurityRuntimeDecorator* guard = nullptr;
    int repairs = 0, verifications = 0;

    Scenario scenario;
    scenario.number = 1;
    scenario.title = "Scenario 1: campus portal recovery";
    scenario.definitionId = "campus-recovery";
    scenario.firstStep = "diagnose";
    scenario.version = 1;
    scenario.budget = 0;

    scenario.define = [&]() {
        demo.store(makeBlueprint("campus-recovery", "Campus portal recovery", 1,
            {{"diagnose", "Diagnose the outage", "technician", "", ""},
             {"repair", "Restore the portal configuration", "administrator", "", ""},
             {"verify", "Test student access", "technician", "", ""},
             {"close", "Close the incident", "administrator", "", ""}},
            {{"diagnose", "repair"}, {"repair", "verify"}}));
        demo.repository.save(demo.repository.load("campus-recovery", 1)->cloneWithVersion(2));
        demo.beat("define", true, "", "Versioning", "v2 is a deep clone of v1, with its own nodes", "Prototype",
                  "definition->cloneWithVersion(2)");
    };

    // Requests pass through Security -> AllowedStates -> Logging -> stage
    scenario.treeWrapper = [&](RuntimeComponent* tree) {
        guard = new SecurityRuntimeDecorator(
            new AllowedStatesRuntimeDecorator(new LoggingRuntimeDecorator(tree), {"IDLE", "RUNNING", "COMPLETED"}),
            "administrator");
        return guard;
    };

    scenario.onStart = [&](const std::string&) {
        guard->setActor(&student);
        try { guard->setState("RUNNING"); }
        catch (const std::runtime_error& error) {
            demo.beat("start", true, "", "Security decorator", std::string("Student refused. ") + error.what(), "Decorator");
        }
        guard->setActor(&bob);
        try { guard->setState("BANANA"); }
        catch (const std::invalid_argument& error) {
            demo.beat("start", true, "", "AllowedStates decorator", std::string("Bob refused. ") + error.what(), "Decorator");
        }
        guard->setState("RUNNING");
        demo.beat("start", true, "", "Logging decorator", "Bob set a valid state, so it reached the stage and was logged", "Decorator");
    };

    scenario.special["repair"] = [&](WorkItem* item) {
        if (++repairs > 1) return demo.startAndComplete(item);
        demo.engine.submit(new StartCommand(item));
        Request incomplete{Data("")};
        Request request("repair-approval", "APPROVE", item->getId(), demo.instanceId, "administrator",
                        "p3", "Restore last known good configuration", item->getState(), Data("Portal outage"));
        validation.authorise(incomplete);
        validation.authorise(request);
        demo.unique("repair", "Approval chain", "Incomplete request stopped at validation; "
                    "valid request passed technician and reached administrator", "Chain of Responsibility", "validation.authorise(request)");
        demo.escalateAndUndo(item);
        demo.engine.submit(new CompleteCommand(item));
        return std::string("approved");
    };

    scenario.special["verify"] = [&](WorkItem* item) {
        std::string outcome = ++verifications == 1 ? "failed" : "passed";
        demo.startAndComplete(item, outcome);
        demo.unique("verify", "Routing loop", "login test " + outcome + (outcome == "failed" ? ", so undo rolls the work back and routing returns to the repair"
                    : ", so routing closes the incident"), "Strategy (routing)", "router->next(step, definition, outcome)");
        return outcome;
    };

    scenario.routers["verify"] = &verification;
    scenario.branches = {{"verify", "close", "passed"}, {"verify", "repair", "failed"}};
    demo.run(scenario);
}

/** @brief Scenario 2: an approval, a nested process, and a deadline that is missed. */
void runProjectApproval(Demo& demo, Participant& lerato, Participant& mandla) {
    std::vector<std::unique_ptr<ApprovalHandler>> approvalChain;
    ConditionalRouting budgetCheck("over_limit", "execApproval", "kickoff");

    auto definitionFor = [](int version) {
        std::vector<Step> steps = {{"submit", "Submit Proposal", "proposer", "", ""},
                                   {"productReview", "Product Review", "product_manager", "Review Stage", ""},
                                   {"techReview", "Technical Review", "head_engineer", "Review Stage", ""}};
        std::vector<std::pair<std::string, std::string>> links = {{"submit", "productReview"}, {"productReview", "techReview"}};
        if (version == 2) {
            steps.push_back({"securityReview", "Security Review", "head_engineer", "Review Stage", ""});
            links.push_back({"techReview", "securityReview"});
            links.push_back({"securityReview", "budgetAudit"});
        } else {
            links.push_back({"techReview", "budgetAudit"});
        }
        steps.push_back({"budgetAudit", "Budget Audit", "", "Budget Stage", "budget-audit"});
        steps.push_back({"financeReview", "Budget Review", "finance", "Budget Stage", ""});
        steps.push_back({"execApproval", "Executive Approval", "executive", "Budget Stage", ""});
        steps.push_back({"kickoff", "Project Kickoff", "proposer", "", ""});
        links.push_back({"budgetAudit", "financeReview"});
        links.push_back({"execApproval", "kickoff"});
        WorkflowBlueprint blueprint = makeBlueprint("project-approval", "Project Approval", version, steps, links);
        ApprovalChainDefinition chain("finance-executive-chain");
        chain.addStep(ApprovalStepDefinition("finance", 86400, "escalate"));
        chain.addStep(ApprovalStepDefinition("executive", 259200, "escalate"));
        blueprint.approvalChains.push_back(chain);
        return blueprint;
    };

    Scenario big;
    big.number = 2;
    big.title = "Scenario 2: project approval, R750 000 on v1";
    big.definitionId = "project-approval";
    big.firstStep = "submit";
    big.calledDefinition = "budget-audit";
    big.version = 1;
    big.budget = 750000;

    big.define = [&]() {
        demo.store(makeBlueprint("budget-audit", "Budget Audit", 1,   // runs as a sub-process of both versions
            {{"compareProjects", "Compare past projects", "analyst", "", ""},
             {"summaryReport", "Write summary report", "analyst", "", ""}},
            {{"compareProjects", "summaryReport"}}));
        demo.store(definitionFor(1));
        demo.store(definitionFor(2));
        demo.beat("define", true, "", "Versioning", "two versions built with stages inside stages; v2 adds a Security Review",
                  "Builder + Composite", "director.construct(builder, blueprint)");
    };

    big.onStart = [&](const std::string& instanceId) {   // the chain comes from the definition, per case
        WorkflowInstance* instance = demo.engine.getInstance(instanceId);
        WorkflowDefinition* definition = demo.repository.load(instance->getDefinitionId(), instance->getDefinitionVersion());
        approvalChain = buildApprovalChain(*definition->getApprovalChainById("finance-executive-chain"),
                                           instance->findWorkItem("execApproval"), &demo.engine);
    };

    big.special["productReview"] = [&](WorkItem* item) {
        demo.engine.submit(new RejectCommand(item));
        bool allowed = demo.engine.submit(new CompleteCommand(item));
        demo.unique("productReview", "Rework", std::string("Lerato sent it back; completing it anyway was ") +
                    (allowed ? "allowed" : "refused by the Rejected state"), "State", "engine.submit(RejectCommand)");
        demo.engine.submit(new makeAvailableCommand(item));
        demo.engine.submit(new AssignCommand(item, &lerato));
        return demo.startAndComplete(item);
    };

    big.special["techReview"] = [&](WorkItem* item) {
        demo.engine.submit(new StartCommand(item));
        demo.escalateAndUndo(item);
        demo.engine.submit(new CompleteCommand(item));
        return std::string("approved");
    };

    big.special["financeReview"] = [&](WorkItem* item) {
        const int budget = std::stoi(demo.engine.getInstance(demo.instanceId)->getData().getValue());
        std::string outcome = budget > EXECUTIVE_LIMIT ? "over_limit" : "within_limit";
        demo.engine.submit(new StartCommand(item));
        Request request("REQ-FIN", "APPROVE", item->getId(), demo.instanceId, "finance", "finance-reviewer",
                        "Budget approval", item->getState(), demo.engine.getInstance(demo.instanceId)->getData());
        approvalChain[0]->authorise(request);
        demo.bus.notify(WorkFlowEvent("APPROVAL_DECISION", demo.instanceId, item->getId(), "budget approved (" + outcome + ")"));
        demo.engine.submit(new CompleteCommand(item));
        if (outcome == "within_limit") {   // routing skips this step, so nobody should still hold it
            demo.engine.submit(new CancelCommand(demo.engine.getInstance(demo.instanceId)->findWorkItem("execApproval")));
        }
        demo.unique("financeReview", "Budget branch", "R" + std::to_string(budget) + " is " +
                    (outcome == "over_limit" ? "above" : "within") + " the R" + std::to_string(EXECUTIVE_LIMIT) +
                    " limit, so routing goes to " + (outcome == "over_limit" ? "the executive" : "kickoff"),
                    "Chain of Responsibility + Strategy (routing)", "approvalChain->authorise(request)");
        return outcome;
    };

    big.special["execApproval"] = [&](WorkItem* item) {
        Request timeout("REQ-TIMEOUT", "TIMEOUT", item->getId(), demo.instanceId, "executive", "system",
                        "Executive approval deadline exceeded", item->getState(),
                        demo.engine.getInstance(demo.instanceId)->getData());
        approvalChain[0]->authorise(timeout);
        demo.unique("execApproval", "Timeout", "Naledi missed the deadline; the handlers passed the request on until the "
                    "timeout handler escalated the item (" + item->getState() + ")", "Chain of Responsibility", "approvalChain->authorise(timeout)");
        demo.engine.submit(new AssignCommand(item, &mandla));
        demo.bus.notify(WorkFlowEvent("APPROVAL_DECISION", demo.instanceId, item->getId(), "approved by the CEO"));
        demo.unique("execApproval", "Escalated", "Mandla (CEO) picked up the approval", "Command + State", "engine.submit(AssignCommand)");
        return demo.startAndComplete(item);
    };

    big.routers["financeReview"] = &budgetCheck;
    big.branches = {{"financeReview", "execApproval", "over_limit"}, {"financeReview", "kickoff", "within_limit"}};

    // A second case: v2 and a smaller budget, so the other branch is taken
    Scenario small = big;
    small.title = "Scenario 2: project approval, R300 000 on v2";
    small.version = 2;
    small.budget = 300000;
    small.define = nullptr;
    small.special.erase("productReview");
    small.special.erase("techReview");
    small.special.erase("execApproval");

    demo.run(big);
    demo.run(small);
}

} // namespace
/** @brief Entry point; runs the EventFlow demo.
 *  @param argc Argument count.
 *  @param argv Argument values.
 *  @return 0 on success. 
 */
int main(int argc, char* argv[]) {
    std::cout << "TIVIDY: one workflow engine, any process\n\n"
              << "How it works\n"
              << "  1. Define  A process is described once and stored as a versioned definition.\n"
              << "  2. Start   Each real case becomes a live tree of work items, assigned to people.\n"
              << "  3. Run     Every action is a command that the item's state allows or refuses.\n"
              << "  4. Route   Routing strategies choose the next step, including loops and branches.\n"
              << "  5. Report  Observers record every change and visitors build the reports.\n\n";

    // The overview stays short: the steps only print with --verbose
    const bool verbose = argc > 1 && std::string(argv[1]) == "--verbose";
    std::ostringstream hidden;
    std::streambuf* console = std::cout.rdbuf();
    if (!verbose) std::cout.rdbuf(hidden.rdbuf());

    Demo demo;
    Participant sam("c1", "Sam", {"technician"}, 1), alice("c2", "Alice", {"technician"}, 1), bob("c3", "Bob", {"administrator"}, 2);
    Participant deon("p1", "Deon", {"proposer"}, 0), lerato("p2", "Lerato", {"product_manager"}, 1);
    Participant sipho("p3", "Sipho", {"head_engineer"}, 2), thandi("p4", "Thandi", {"head_engineer"}, 2);
    Participant ayesha("p5", "Ayesha", {"finance"}, 2), naledi("p6", "Naledi", {"executive"}, 3), mandla("p7", "Mandla", {"executive"}, 4);
    Participant zanele("p8", "Zanele", {"analyst"}, 1);
    for (Participant* busy : {&sam, &sipho}) { busy->addOpenItem(); busy->addOpenItem(); }

    WorkloadBasedAssignment assignment;
    demo.engine.setAssignment(&assignment, {&sam, &alice, &bob, &deon, &lerato, &sipho, &thandi, &ayesha, &naledi, &mandla, &zanele});

    runCampusRecovery(demo, bob);
    runProjectApproval(demo, lerato, mandla);

    const std::vector<Comparison> comparison = {
        {"Run loop", "One loop walks every process", "One loop walks every process", true},
        {"Starting a case", "The same startWorkflow call", "The same startWorkflow call", true},
        {"Doing work", "Every action is a command sent to the engine", "Every action is a command sent to the engine", true},
        {"Assignment", "The least busy person with the right role", "The least busy person with the right role", true},
        {"Rollback", "Undo restores the earlier state", "Undo restores the earlier state", true},
        {"Reports", "The same progress and workload reports", "The same progress and workload reports", true},
        {"Process shape", "Flat: four activities", "Nested: stages inside the root", false},
        {"Sub-process", "None", "Budget Audit runs as its own workflow", false},
        {"Versioning", "Prototype: v2 cloned from v1", "Builder: v2 built with a Security Review", false},
        {"Tree decorators", "Security, AllowedStates, Logging, Label", "Label only", false},
        {"Routing branch", "Loop back to repair when a test fails", "Skip executive when the budget is small", false},
        {"Approval chain", "Validation, technician, administrator", "Finance, executive, timeout", false},
        {"What goes wrong", "Unauthorised update and a failed test", "Rejection and a missed deadline", false},
    };

    std::cout.rdbuf(console);
    std::cout << (verbose ? "\n" : "") << "Just ran on one engine\n";
    for (const std::string& line : demo.summary) std::cout << "  " << line << "\n";

    if (!demo.trace.write("gui/trace.js", comparison)) std::cerr << "Could not write gui/trace.js\n";
    std::cout << "\nSee it run: open gui/index.html\n";
    return 0;
}
