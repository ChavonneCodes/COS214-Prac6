/**
 * @file main.cpp
 * @brief TIVIDY demo: one project proposal at a tech company, run through the whole system.
 *
 * A developer submits a project proposal. It is reviewed by the product
 * manager, then by a head software engineer, then finance checks the budget.
 * Budgets above the executive limit also need executive sign-off before the
 * project can kick off.
 *
 * The output is written for someone seeing the system for the first time:
 * it explains what a workflow engine is, tells the story one step at a time,
 * and says which design pattern is doing the work at each point. It ends with
 * a table of every pattern shown and where to find it.
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
const size_t LOG_PREVIEW = 6;         ///< How many audit and history lines to show before "...".

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
 * @brief Prints plain-English lines that explain what is about to happen.
 * @param lines Lines to print, each indented by two spaces.
 */
void explain(const std::vector<std::string>& lines) {
    for (const std::string& line : lines) std::cout << "  " << line << "\n";
}

/**
 * @brief Prints which design pattern is doing the work, and why.
 * @param name What the pattern is called.
 * @param why  One sentence on what it solves at this point of the story.
 */
void pattern(const std::string& name, const std::string& why) {
    const size_t width = 80;
    std::string line = "  [Pattern: " + name + "]";
    std::string word;
    for (size_t i = 0; i <= why.size(); ++i) {
        if (i < why.size() && why[i] != ' ') {
            word += why[i];
            continue;
        }
        if (word.empty()) continue;
        if (line.size() + 1 + word.size() > width) {   // wrap, keeping the text lined up
            std::cout << line << "\n";
            line = "      " + word;
        } else {
            line += " " + word;
        }
        word.clear();
    }
    std::cout << line << "\n";
}

/**
 * @brief Prints a list, cut short after LOG_PREVIEW lines.
 * @param lines The full list.
 */
void printPreview(const std::vector<std::string>& lines) {
    for (size_t i = 0; i < lines.size() && i < LOG_PREVIEW; ++i) std::cout << "    " << lines[i] << "\n";
    if (lines.size() > LOG_PREVIEW) {
        std::cout << "    ... (" << lines.size() - LOG_PREVIEW << " more, " << lines.size() << " in total)\n";
    }
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
 * @brief Submits a command through the engine and prints what happened.
 *
 * The action is printed first, then the result: the new state if the work
 * item's state allowed it, or "refused" if it did not.
 * @param engine        The engine (takes ownership of the command).
 * @param who           Person doing the action.
 * @param action        What they do, in plain words.
 * @param command       Command to run.
 * @param item          Work item the command acts on.
 * @param expectRefusal true if this is a deliberate illegal action, to show the State pattern refusing it.
 * @return true if the work item's state allowed the action.
 */
bool act(WorkflowEngine& engine, const std::string& who, const std::string& action,
         WorkflowAction* command, WorkItem* item, bool expectRefusal = false) {
    std::string before = item->getCurrentState()->getName();
    std::cout << "  " << who << ": " << action
              << (expectRefusal ? " (should be refused)" : "") << "\n";
    bool ok = engine.submit(command);
    if (ok) {
        std::cout << "      " << before << " -> " << item->getCurrentState()->getName() << "\n";
    } else {
        std::cout << "      refused: not allowed while the item is " << before
                  << (expectRefusal ? ", as expected" : "") << "\n";
    }
    return ok;
}

/**
 * @brief Runs the normal path for an assigned work item: start, then complete.
 * @param engine The engine.
 * @param item   Work item to work on (already assigned by the factory).
 */
void startAndComplete(WorkflowEngine& engine, WorkItem* item) {
    std::string who = item->getAssignedParticipant()->getName();
    act(engine, who, "start " + item->getName(), new StartCommand(item), item);
    act(engine, who, "complete " + item->getName(), new CompleteCommand(item), item);
}

} // namespace

int main() {
    heading("TIVIDY: A WORKFLOW ENGINE");
    explain({
        "TIVIDY runs business processes. A company describes a process once, as a",
        "workflow DEFINITION: its steps, which role may do each step, and the order.",
        "The engine then runs any number of INSTANCES of that definition, one for each",
        "real case. Each step of a running instance is a WORK ITEM. A work item is",
        "given to a person and moves through a life cycle:",
        "",
        "    Created -> Available -> Assigned -> In_Progress -> Completed",
        "    (it can also be Rejected, Escalated, Cancelled or Failed on the way)",
        "",
        "Every action is checked against that life cycle, recorded, and can be undone.",
        "",
        "THE STORY IN THIS DEMO",
        "Deon, a developer, proposes a new project with a budget of R" + std::to_string(PROPOSAL_BUDGET) + ".",
        "It needs a product review (Lerato), a technical review (a head engineer) and",
        "a budget review (Ayesha, finance). Because the budget is above R" + std::to_string(EXECUTIVE_LIMIT) + ",",
        "an executive must also sign off before the project can kick off.",
        "Three things go wrong on the way, to show how the engine copes:",
        "  1. the product review is sent back once,",
        "  2. a reviewer escalates by mistake and it is rolled back,",
        "  3. the executive misses the deadline, so the approval is escalated to the CEO.",
        "",
        "Lines starting with [Pattern: ...] say which design pattern is doing the work.",
        "A table of every pattern and where it appears is printed at the end."
    });

    // ---------------------------------------------------------------
    heading("1. SET UP THE ENGINE");
    InMemoryDefinitionRepository repository;
    EventBus eventBus;
    HistoryManager history;
    AuditObserver audit;
    NotificationObserver decisions("APPROVAL_DECISION");
    eventBus.attach(&audit);
    eventBus.attach(&decisions);
    WorkflowEngine engine(&repository, &eventBus, &history, &audit);

    explain({"The engine is created with a place to store definitions, an event bus,",
             "a command history and an audit log."});
    pattern("Facade", "everything below goes through one object, WorkflowEngine. The client never "
                      "touches the repository, factory, history or audit log directly.");
    pattern("Observer", "the engine publishes every change on the event bus. " +
                        std::to_string(eventBus.observerCount()) + " observers listen: an audit log "
                        "that records every state change, and a notifier that tells the proposer "
                        "about approval decisions.");

    // ---------------------------------------------------------------
    heading("2. DEFINE THE PROCESS (the template)");
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
    explain({"The Project Approval process is defined and saved in two versions.",
             "Version 2 adds a Security Review. A running case keeps the version it started",
             "on, so changing the process never breaks cases that are already in progress."});
    pattern("Builder", "a Director feeds the steps to a Builder one at a time, so a "
                       "definition is never half-built.");
    pattern("Composite", "the process is a tree: stages (+) contain activities (-) or other "
                         "stages, and the engine treats single steps and whole stages the same way.");
    std::cout << "\n  The repository holds " << repository.findAll().size() << " versions of '"
              << latest->getName() << "'. Latest is v" << latest->getVersion() << ":\n";
    latest->getRoot()->print(2);

    // ---------------------------------------------------------------
    heading("3. START A CASE (an instance of the process)");
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

    explain({"First, a mistake: asking for a process that does not exist."});
    std::string missing = engine.startWorkflow("does-not-exist", 1);
    std::cout << "    " << (missing.empty() ? "refused: no such definition, nothing was started"
                                            : "started " + missing) << "\n\n";

    explain({"Now Deon's proposal is started on version 1. The engine copies the template",
             "into a live tree of work items and gives each one to a person. Sipho already",
             "has two open items, so the technical review goes to Thandi instead:"});
    std::string instanceId = engine.startWorkflow(DEFINITION_ID, 1);
    WorkflowInstance* instance = engine.getInstance(instanceId);
    if (!instance) {
        std::cerr << "ERROR: the engine could not start the workflow.\n";
        return 1;
    }
    instance->getData().setValue(std::to_string(PROPOSAL_BUDGET));
    WorkflowDefinition* definition = repository.load(DEFINITION_ID, instance->getDefinitionVersion());

    pattern("Factory Method", "InstanceFactory turns each definition node into the matching runtime "
                              "node (activity -> work item, stage -> stage instance).");
    pattern("Strategy", "who gets each work item is decided by a swappable assignment strategy (" +
                        assignment.getName() + ": the least busy person with the right role).");
    pattern("Decorator", "the runtime tree is wrapped with a label showing the instance id and "
                         "the version it is pinned to, without changing the tree classes.");
    std::cout << "\n  Case " << instanceId << " is " << instance->getState() << " on v"
              << instance->getDefinitionVersion() << ".\n  It has no Security Review, because that is only in v"
              << latest->getVersion() << ". Live tree:\n";
    instance->getRoot()->print(2);
    explain({"(Stages only group work items, so they stay IDLE. Work items carry the state.)"});

    // ---------------------------------------------------------------
    // Chain of Responsibility: finance -> executive -> timeout escalation.
    // The timeout handler watches the executive approval work item.
    RoleApprovalHandler financeApprover("finance");
    RoleApprovalHandler executiveApprover("executive");
    TimeoutEscalationHandler timeoutEscalation(instance->findWorkItem("execApproval"));
    financeApprover.setNext(&executiveApprover);
    executiveApprover.setNext(&timeoutEscalation);

    // Routing strategies: sequential by default, budget check after finance review
    SequentialRouting sequential;
    ConditionalRouting budgetCheck("over_limit", "execApproval", "kickoff");
    std::map<std::string, RoutingStrategy*> routingFor;
    routingFor["financeReview"] = &budgetCheck;

    // What each step is about, printed before it runs.
    std::map<std::string, std::string> story;
    story["submit"] = "Deon submits the proposal.";
    story["productReview"] = "Lerato finds the scope unclear and sends the proposal back. Deon fixes it.";
    story["techReview"] = "Thandi reviews the architecture, but escalates the review by mistake.";
    story["financeReview"] = "Ayesha asks the approval chain to sign off the budget.";
    story["execApproval"] = "Naledi misses the approval deadline, so the request is escalated.";
    story["kickoff"] = "Everything is approved. Deon kicks off the project.";

    // What happens inside each activity. Each returns the outcome used for routing.
    std::map<std::string, std::function<std::string(WorkItem*)>> behaviour;

    behaviour["submit"] = [&](WorkItem* item) {
        pattern("Command", "each action below is a command object handed to the engine, which "
                           "runs it and keeps it in the history so it can be undone.");
        pattern("State", "the work item's current state object decides whether an action is allowed.");
        startAndComplete(engine, item);
        return std::string("approved");
    };

    behaviour["productReview"] = [&](WorkItem* item) {
        act(engine, "Lerato", "send the proposal back", new RejectCommand(item), item);
        act(engine, "Lerato", "complete the rejected review anyway", new CompleteCommand(item), item, true);
        pattern("State", "a Rejected item cannot be completed, so the state object refused it "
                         "(and printed \"Invalid State Transition\").");
        std::cout << "  Deon clarifies the scope, so the review is offered again.\n";
        item->makeAvailable();
        act(engine, "Lerato", "take the review again", new AssignCommand(item, &lerato), item);
        startAndComplete(engine, item);
        return std::string("approved");
    };

    behaviour["techReview"] = [&](WorkItem* item) {
        act(engine, "Thandi", "start Technical Review", new StartCommand(item), item);

        // Memento + Prototype: snapshot the item before anything risky
        Data notes("Architecture review: microservices design accepted");
        WorkItemMemento snapshot("techReview", item->getId(), notes, item->getCurrentState());
        std::cout << "  Before going on, a snapshot of the review is saved (state "
                  << item->getCurrentState()->getName() << ").\n";
        pattern("Memento", "the snapshot stores the work item's state and notes outside the item, "
                           "so it can be put back later.");
        pattern("Prototype", "the snapshot holds a clone of the state object, so later changes to "
                             "the item cannot change the saved copy.");

        act(engine, "Thandi", "escalate to the executives (by mistake)",
            new EscalateCommand(item), item);

        std::cout << "  Thandi notices the mistake. Rolling back:\n";
        engine.undoLast();
        item->restoreMemento(&snapshot);
        std::cout << "      state restored to " << item->getCurrentState()->getName()
                  << ", notes kept: \"" << notes.getValue() << "\"\n";
        pattern("Command + Memento", "the engine takes the escalation off its history (undo), and "
                                     "the memento restores the state from before it.");

        act(engine, "Thandi", "complete Technical Review", new CompleteCommand(item), item);
        return std::string("approved");
    };

    behaviour["financeReview"] = [&](WorkItem* item) {
        act(engine, "Ayesha", "start Budget Review", new StartCommand(item), item);

        std::cout << "  Approval request for role 'finance' sent to the chain "
                     "(finance -> executive -> timeout):\n";
        Request request("REQ-FIN-001", "APPROVE", item->getId(), instanceId, "finance",
                        "finance-reviewer", "Budget approval", item->getCurrentState()->getName(),
                        instance->getData());
        HandlerResult financeResult = financeApprover.authorise(request);
        pattern("Chain of Responsibility", "the request goes along a chain of handlers until one "
                                           "of them can approve it, so the sender does not need "
                                           "to know who approves what.");

        if (financeResult != HandlerResult::Handled) {
            std::cout << "  The budget was not approved, so the case stops here.\n";
            return std::string("rejected");
        }

        std::string outcome = std::stoi(instance->getData().getValue()) > EXECUTIVE_LIMIT
                                  ? "over_limit" : "within_limit";
        eventBus.notify(WorkFlowEvent("APPROVAL_DECISION", instanceId, item->getId(),
                                      "budget approved by finance (" + outcome + ")"));
        act(engine, "Ayesha", "complete Budget Review", new CompleteCommand(item), item);
        std::cout << "  Budget R" << PROPOSAL_BUDGET << " is "
                  << (outcome == "over_limit" ? "above" : "within") << " the R" << EXECUTIVE_LIMIT
                  << " limit (outcome '" << outcome << "').\n";
        return outcome;
    };

    behaviour["execApproval"] = [&](WorkItem* item) {
        std::cout << "  The deadline passes without a decision from Naledi.\n"
                     "  A timeout request goes to the start of the chain:\n";
        Request timeout("REQ-TIMEOUT-001", "TIMEOUT", item->getId(), instanceId, "executive",
                        "system", "Executive approval deadline exceeded",
                        item->getCurrentState()->getName(), instance->getData());
        financeApprover.authorise(timeout);
        std::cout << "      Assigned -> " << item->getCurrentState()->getName() << "\n";
        pattern("Chain of Responsibility", "both approval handlers passed the timeout on; only the "
                                           "timeout handler deals with it, and it escalates the item.");

        act(engine, "Mandla (CEO)", "pick up the escalated approval", new AssignCommand(item, &mandla), item);

        std::cout << "  Approval request for role 'executive' sent to the chain:\n";
        Request signOff("REQ-EXEC-001", "APPROVE", item->getId(), instanceId, "executive",
                        "executive-reviewer", "Executive project sign-off",
                        item->getCurrentState()->getName(), instance->getData());
        if (financeApprover.authorise(signOff) != HandlerResult::Handled) {
            std::cout << "  The executive did not approve, so the case stops here.\n";
            return std::string("rejected");
        }
        eventBus.notify(WorkFlowEvent("APPROVAL_DECISION", instanceId, item->getId(),
                                      "project approved by the executive"));
        startAndComplete(engine, item);
        return std::string("approved");
    };

    // ---------------------------------------------------------------
    heading("4. RUN THE CASE, ONE STEP AT A TIME");
    explain({"Each step shows what happens, who does what, the result, and what comes next."});

    int stepNumber = 0;
    bool firstStep = true;
    std::string currentId = "submit";
    while (!currentId.empty()) {
        WorkItem* item = instance->findWorkItem(currentId);
        Participant* who = item->getAssignedParticipant();
        std::cout << "\n--- Step " << ++stepNumber << ": " << item->getName() << " ("
                  << (who ? who->getName() : "unassigned") << ", role " << item->getRequiredRoleId()
                  << ") ---\n";
        if (story.count(currentId)) std::cout << "  " << story[currentId] << "\n";

        std::string outcome = "approved";
        if (behaviour.count(currentId)) {
            outcome = behaviour[currentId](item);
        } else {
            startAndComplete(engine, item);
        }
        if (outcome == "rejected") break;

        const ActivityDefinition* activity = findActivity(*definition, currentId);
        RoutingStrategy* routing = routingFor.count(currentId) ? routingFor[currentId] : &sequential;
        std::vector<const WorkflowComponent*> next = routing->next(*activity, *definition, outcome);
        currentId = next.empty() ? "" : next[0]->getId();

        WorkItem* nextItem = currentId.empty() ? nullptr : instance->findWorkItem(currentId);
        std::cout << "  Next: " << (nextItem ? nextItem->getName() : "end of the process")
                  << "  (decided by " << routing->getName() << " routing)\n";
        if (firstStep) {
            pattern("Strategy", "what comes next is decided by a swappable routing strategy. Most "
                                "steps use Sequential routing; the budget review uses Conditional "
                                "routing.");
        }
        if (routing == &budgetCheck) {
            pattern("Strategy", "Conditional routing sent the case to Executive Approval because the "
                                "outcome was 'over_limit'. A smaller budget would go straight to kickoff.");
        }

        // Visitor: a fresh progress visitor walks the whole tree after every step
        ProgressReportVisitor progress;
        engine.applyVisitor(instanceId, progress);
        std::cout << "  Progress: " << progress.getCompletedItems() << " of " << progress.getTotalItems()
                  << " steps done (" << progress.getPercentComplete() << "%)\n";
        if (firstStep) {
            pattern("Visitor", "the progress line is worked out by a visitor that walks the live tree "
                               "and counts finished work items.");
        }
        firstStep = false;
    }
    instance->setState("COMPLETED");

    // ---------------------------------------------------------------
    heading("5. WHAT THE ENGINE RECORDED");
    explain({"Everything here is read back through the engine (the Facade)."});
    std::cout << "\n  Case " << instanceId << " is " << engine.getInstance(instanceId)->getState()
              << ". Final tree\n  (note that Mandla, not Naledi, did the executive approval):\n";
    engine.getInstance(instanceId)->getRoot()->print(2);

    std::cout << "\n  Notifications sent to the proposer (Observer, filtered to approval decisions):\n";
    for (const std::string& line : decisions.getNotifications()) std::cout << "    " << line << "\n";

    std::vector<std::string> trail = engine.getAuditTrail(instanceId);
    std::cout << "\n  Audit trail: every state change, published on the event bus (Observer):\n";
    printPreview(trail);
    std::cout << "  The Technical Review entries still show the escalation that was rolled back,\n"
              << "  because an audit log never forgets:\n";
    for (const std::string& line : trail) {
        if (line.find("item=techReview ") != std::string::npos) std::cout << "    " << line << "\n";
    }

    std::vector<std::string> commands;
    for (WorkflowAction* action : engine.getHistory(instanceId)) commands.push_back(action->getDescription());
    std::cout << "\n  Command history: the commands that are still in effect (Command).\n"
              << "  The undone escalation is not here, which is what makes undo possible:\n";
    printPreview(commands);

    // ---------------------------------------------------------------
    heading("6. REPORTS");
    pattern("Visitor", "two new reports over the live tree, added without changing WorkItem, "
                       "StageInstance or SubProcessInstance. The engine passes each visitor to the tree.");

    ProgressReportVisitor report;
    engine.applyVisitor(instanceId, report);
    std::cout << "\n  Completion report for " << instanceId << ":\n";
    for (const std::string& line : report.getReport()) std::cout << "    " << line << "\n";
    std::cout << "    Process complete: " << (report.isComplete() ? "yes" : "no") << "\n";

    WorkloadVisitor workload;
    engine.applyVisitor(instanceId, workload);
    std::cout << "\n  Who did what:\n";
    for (const std::string& line : workload.getSummary()) std::cout << "    " << line << "\n";

    // ---------------------------------------------------------------
    heading("PATTERNS IN THIS DEMO");
    std::cout
        << "  Pattern                  Where you saw it                      Main classes\n"
        << "  -----------------------  ------------------------------------  ------------------------------------\n"
        << "  Facade                   every action goes through the engine  WorkflowEngine\n"
        << "  Observer                 audit trail and notifications         EventBus, AuditObserver,\n"
        << "                                                                 NotificationObserver\n"
        << "  Builder                  section 2, building the definition    WorkflowDirector,\n"
        << "                                                                 ConcreteWorkflowBuilder\n"
        << "  Composite                the definition and live trees         StageDefinition, ActivityDefinition,\n"
        << "                                                                 StageInstance, WorkItem\n"
        << "  Factory Method           section 3, starting the case          InstanceFactory\n"
        << "  Strategy                 who does a step, and what comes next  WorkloadBasedAssignment,\n"
        << "                                                                 SequentialRouting, ConditionalRouting\n"
        << "  Decorator                the label on the live tree            LabelledRuntimeDecorator\n"
        << "  Command                  every action in section 4, and undo   AssignCommand, StartCommand, ...,\n"
        << "                                                                 HistoryManager\n"
        << "  State                    actions allowed or refused            WorkItemState and its subclasses\n"
        << "  Chain of Responsibility  steps 4 and 5, approvals and timeout  RoleApprovalHandler,\n"
        << "                                                                 TimeoutEscalationHandler\n"
        << "  Memento                  step 3, rolling back the escalation   WorkItemMemento\n"
        << "  Prototype                step 3, cloning the saved state       WorkItemState::clone()\n"
        << "  Visitor                  progress lines and section 6 reports  RuntimeVisitor, ProgressReportVisitor,\n"
        << "                                                                 WorkloadVisitor\n";

    heading("DEMO COMPLETE");
    return 0;
}
