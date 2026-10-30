/**
 * Campus portal recovery: a narrated demonstration of the implemented patterns.
 * Demonstrates Security, AllowedStates and Logging runtime decorators.
 * The client drives routing; people performing repairs are simulated by commands.
 */
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>
#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "ConcreteWorkflowBuilder.h"
#include "InMemoryDefinitionRepository.h"
#include "WorkflowEngine.h"
#include "InstanceFactory.h"
#include "StageInstance.h"
#include "StageDefinition.h"
#include "WorkItem.h"
#include "SecurityRuntimeDecorator.h"
#include "AllowedStatesRuntimeDecorator.h"
#include "LoggingRuntimeDecorator.h"
#include "WorkloadBasedAssignment.h"
#include "SequentialRouting.h"
#include "ConditionalRouting.h"
#include "RoutingHelper.h"
#include "ValidationApprovalHandler.h"
#include "RoleApprovalHandler.h"
#include "AssignCommand.h"
#include "StartCommand.h"
#include "CompleteCommand.h"
#include "EscalateCommand.h"
#include "ProgressReportVisitor.h"
#include "WorkloadVisitor.h"
#include "NotificationObserver.h"

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error("Check failed: " + message);
    std::cout << "  [OK] " << message << '\n';
}
void section(const std::string& title) {
    std::cout << "\n============================================================\n"
              << title << "\n============================================================\n";
}
void pattern(const std::string& name, const std::string& purpose) {
    std::cout << "[Pattern: " << name << "] " << purpose << '\n';
}

WorkflowBlueprint makeBlueprint() {
    WorkflowBlueprint plan;
    plan.id = "campus-recovery";
    plan.name = "Restore the campus learning portal";
    plan.roles.push_back(RoleDefinition("technician", "Support technician", 1));
    plan.roles.push_back(RoleDefinition("administrator", "Service administrator", 2));
    plan.activities.push_back(ActivityDefinition("diagnose", "Diagnose the outage", "technician"));
    plan.activities.push_back(ActivityDefinition("repair", "Restore the portal configuration", "administrator"));
    plan.activities.push_back(ActivityDefinition("verify", "Test student access", "technician"));
    plan.activities.push_back(ActivityDefinition("close", "Close the incident", "administrator"));
    plan.transitions.push_back(TransitionDefinition("diagnose", "repair", "always"));
    plan.transitions.push_back(TransitionDefinition("repair", "verify", "always"));
    // Verification branches through ConditionalRouting: passed -> close,
    // failed -> repair. The story below deliberately takes both branches.
    plan.hasRootTree = true;
    plan.rootTreeData.type = NodeType::STAGE;
    plan.rootTreeData.id = "recovery";
    plan.rootTreeData.name = "Campus portal recovery";
    for (const ActivityDefinition& activity : plan.activities) {
        WorkflowNodeData node;
        node.type = NodeType::ACTIVITY;
        node.id = activity.getId();
        node.name = activity.getName();
        node.roleId = activity.getRequiredRoleId();
        plan.rootTreeData.children.push_back(node);
    }
    return plan;
}

// Commands retain raw WorkItem receivers, so they bypass decorator methods.
// The scenario explicitly checks the task's security wrapper before submission.
bool submit(WorkflowEngine& engine, SecurityRuntimeDecorator& guard,
            WorkItem& item, std::unique_ptr<WorkflowAction> command) {
    require(guard.canModify(), "Actor has permission for " + item.getId());
    const std::string before = item.getState();
    const bool ok = engine.submit(command.release()); // engine takes ownership
    std::cout << "  " << item.getId() << ": " << before << " -> " << item.getState()
              << (ok ? "\n" : " (action refused)\n");
    return ok;
}
} // namespace

int main() {
    try {
        section("SCENARIO 1: STUDENTS CANNOT ACCESS THE LEARNING PORTAL");
        std::cout << "Alice diagnoses and tests; Bob repairs and closes the incident.\n"
                     "The first verification fails, so the repair must be repeated.\n"
                     "A student attempts an unauthorized update, and an accidental\n"
                     "escalation is restored from a snapshot.\n";

        section("1. Describe the process: Builder + Composite + Prototype");
        ConcreteWorkflowBuilder builder;
        WorkflowDirector director;
        WorkflowBlueprint blueprint = makeBlueprint();
        director.construct(builder, blueprint);
        InMemoryDefinitionRepository repository;
        repository.save(new WorkflowDefinition(builder.build()));
        WorkflowDefinition* definition = repository.load("campus-recovery", 1);
        require(definition != nullptr, "Definition v1 is stored");
        pattern("Builder", "Director constructs a definition from a blueprint.");
        pattern("Composite", "One stage contains four activity leaves.");
        definition->getRoot()->print();
        repository.save(definition->cloneWithVersion(2));
        WorkflowDefinition* copy = repository.load("campus-recovery", 2);
        pattern("Prototype", "cloneWithVersion deep-copies the definition tree.");
        require(copy && copy->getRoot() != definition->getRoot() &&
                copy->getRoot()->getChildren()[0] != definition->getRoot()->getChildren()[0],
                "Version 2 owns independent nodes; this case still uses v1");

        section("2. Build the live case: Factory + Strategy + Decorator");
        Participant student("p0", "Student", {"guest"});
        Participant busy("p1", "Sam", {"technician"}, 1);
        Participant alice("p2", "Alice", {"technician"}, 1);
        Participant bob("p3", "Bob", {"administrator"}, 2);
        busy.addOpenItem(); busy.addOpenItem();
        std::vector<Participant*> staff = {&student, &busy, &alice, &bob};
        WorkloadBasedAssignment assignment;
        EventBus bus;
        AuditObserver audit;
        NotificationObserver notifications("STATE_CHANGED");
        bus.attach(&audit); bus.attach(&notifications);
        HistoryManager history;
        const std::string instanceId = "portal-incident-001";
        auto* stageDefinition = dynamic_cast<StageDefinition*>(definition->getRoot());
        require(stageDefinition != nullptr, "Definition root is a stage");
        auto* stage = new StageInstance(instanceId, stageDefinition);
        // Requests pass through Security -> AllowedStates -> Logging -> Stage.
        // Unauthorized or invalid updates stop before reaching the logger.
        std::unique_ptr<SecurityRuntimeDecorator> root(new SecurityRuntimeDecorator(
            new AllowedStatesRuntimeDecorator(
                new LoggingRuntimeDecorator(stage), {"IDLE", "RUNNING", "COMPLETED"}),
            "administrator", &bob));
        std::map<std::string, WorkItem*> items; // borrowed from the owned tree
        std::map<std::string, SecurityRuntimeDecorator*> guards;
        for (WorkflowComponent* node : definition->getRoot()->getChildren()) {
            // Defer assignment until a task is reached, so dependencies stay explicit.
            std::unique_ptr<RuntimeComponent> created(
                InstanceFactory::instantiateTree(instanceId, node, nullptr, staff));
            auto* item = dynamic_cast<WorkItem*>(created.get());
            require(item != nullptr, "Factory created WorkItem " + node->getId());
            item->setEventBus(&bus);
            std::unique_ptr<SecurityRuntimeDecorator> guard(new SecurityRuntimeDecorator(
                created.release(), item->getRequiredRoleId()));
            root->addChild(guard.get());
            items[item->getId()] = item;
            guards[item->getId()] = guard.release();
        }
        SecurityRuntimeDecorator* stageGuard = root.get();
        // Build locally to avoid startWorkflow's automatic Labelled decorator.
        // WorkflowInstance owns the tree; WorkflowEngine owns submitted commands.
        WorkflowInstance instance(instanceId, definition->getDefinitionId(), 1, root.release(), &bus);
        WorkflowEngine engine(&repository, &bus, &history, &audit);
        pattern("Factory", "InstanceFactory creates tasks from activity definitions.");
        pattern("Strategy", "Least-busy eligible staff are selected when work becomes ready.");
        pattern("Facade", "WorkflowEngine submits commands and retrieves history/audit trails.");
        std::cout << "  The stage uses Security, AllowedStates and Logging decorators.\n";

        section("3. Decorators: check access, validate state names, log updates");
        pattern("Decorator", "Security -> AllowedStates -> Logging -> recovery stage.");
        std::cout << "  Security checks the actor; AllowedStates checks the requested name;\n"
                     "  Logging prints successful updates. Commands on raw tasks bypass these wrappers.\n";
        stageGuard->setActor(&student);
        bool denied = false;
        try { stageGuard->setState("RUNNING"); }
        catch (const std::runtime_error& error) {
            denied = true;
            std::cout << "  Expected refusal: " << error.what() << '\n';
        }
        require(denied && stage->getState() == "IDLE",
                "Unauthorized access leaves the stage unchanged");
        stageGuard->setActor(&bob);
        bool invalidStateRejected = false;
        try { stageGuard->setState("BANANA"); }
        catch (const std::invalid_argument& error) {
            invalidStateRejected = true;
            std::cout << "  Expected validation refusal: " << error.what() << '\n';
        }
        require(invalidStateRejected && stage->getState() == "IDLE",
                "AllowedStates rejects an unknown state without changing the stage");
        std::cout << "  Bob requests a valid state; Logging prints the change below.\n";
        stageGuard->setState("RUNNING");
        require(stageGuard->getState() == "RUNNING",
                "Administrator is allowed to start the stage");
        instance.setState("RUNNING");

        section("4. Inspect the runtime tree: Iterator");
        pattern("Iterator", "A DFS iterator visits tasks through their security wrappers.");
        std::unique_ptr<RuntimeComponentIterator> iterator(instance.getRoot()->createIterator(
            new DFSTraversalStrategy<RuntimeComponent>())); // iterator owns strategy
        int visited = 0;
        while (iterator->hasNext()) {
            const auto& component = dynamic_cast<const RuntimeComponent&>(iterator->getNext());
            std::cout << "  " << component.getId() << ": " << component.getState() << '\n';
            ++visited;
        }
        require(visited == 4, "Iterator visits all four tasks (root is excluded by this iterator)");

        section("5. Run tasks: Command + State + Observer + routing Strategy");
        pattern("Command", "Assign, Start and Complete objects are submitted through the engine.");
        pattern("State", "Each task's state object accepts or refuses lifecycle actions.");
        pattern("Observer", "Audit and notifications receive every task state change.");
        SequentialRouting sequential;
        ConditionalRouting verification("passed", "close", "repair");
        ValidationApprovalHandler validation;
        RoleApprovalHandler technicianApproval("technician");
        RoleApprovalHandler administratorApproval("administrator");
        validation.setNext(&technicianApproval);
        technicianApproval.setNext(&administratorApproval);
        std::string currentId = "diagnose";
        std::vector<std::string> completed;
        int verificationRuns = 0;
        bool restored = false;
        while (!currentId.empty()) {
            require(completed.size() < 10, "Routing stays within the scenario's progress limit");
            WorkItem& item = *items.at(currentId);
            SecurityRuntimeDecorator& guard = *guards.at(currentId);
            Participant* actor = assignment.assign(item, staff);
            require(actor != nullptr, "Assignment strategy finds an eligible participant");
            guard.setActor(actor);
            std::cout << "\n  NEXT: " << item.getName() << " -- " << actor->getName() << '\n';
            // This client's failed verification reopens tasks explicitly. The engine
            // does not automatically reset completed tasks on a routing loop.
            if (item.getState() == "Completed") guard.setState("Created");
            require(item.makeAvailable(), "Reached task becomes Available");
            require(submit(engine, guard, item, std::unique_ptr<WorkflowAction>(
                new AssignCommand(&item, actor))), "Assignment command succeeds");
            if (currentId == "diagnose") {
                const size_t before = history.getAll().size();
                require(!submit(engine, guard, item, std::unique_ptr<WorkflowAction>(
                    new CompleteCommand(&item))), "Completion before starting is refused");
                require(history.getAll().size() == before, "Refused command is excluded from history");
                require(actor == &alice, "Alice is selected instead of busy technician Sam");
            }
            require(submit(engine, guard, item, std::unique_ptr<WorkflowAction>(
                new StartCommand(&item))), "Start command succeeds");
            if (currentId == "repair" && !restored) {
                pattern("Chain of Responsibility", "Validation -> technician -> administrator handles repair approval.");
                Request invalid(Data(""));
                require(validation.authorise(invalid) == HandlerResult::Rejected,
                        "Incomplete approval request is rejected");
                Request request("repair-approval", "APPROVE", item.getId(), instanceId,
                                "administrator", actor->getParticipantId(), "Restore last known good configuration",
                                item.getState(), Data("Portal outage"));
                require(validation.authorise(request) == HandlerResult::Handled,
                        "Valid request passes technician handler and reaches administrator");
                pattern("Memento + Prototype", "Snapshot clones the state before an accidental escalation.");
                WorkItemMemento snapshot = item.createMemento();
                require(snapshot.getStateKind() != item.getCurrentState(), "Snapshot owns a cloned state");
                std::string wrongRole = "management";
                require(submit(engine, guard, item, std::unique_ptr<WorkflowAction>(
                    new EscalateCommand(&item, wrongRole))), "Accidental escalation is recorded");
                require(snapshot.getStateKind()->getName() == "In_Progress",
                        "Snapshot remains unchanged after escalation");
                // Existing undo only prints; restoration must explicitly use Memento.
                engine.undoLast();
                item.restoreMemento(&snapshot);
                require(item.getState() == "In_Progress", "Memento restores the running task");
                restored = true;
            }
            require(submit(engine, guard, item, std::unique_ptr<WorkflowAction>(
                new CompleteCommand(&item))), "Completion command succeeds");
            completed.push_back(currentId);
            std::string outcome = "done";
            RoutingStrategy* router = &sequential;
            if (currentId == "verify") {
                outcome = ++verificationRuns == 1 ? "failed" : "passed";
                router = &verification;
                std::cout << "  Student login test: " << outcome << '\n';
            }
            const WorkflowComponent* definitionNode = findNode(*definition, currentId);
            require(definitionNode != nullptr, "Completed definition node exists");
            const auto next = router->next(*definitionNode, *definition, outcome);
            currentId = next.empty() ? "" : next.front()->getId();
            std::cout << "  " << router->getName() << " routes to: "
                      << (currentId.empty() ? "end of process" : currentId) << '\n';
        }

        section("6. Reports and final checks: Visitor + Facade");
        pattern("Visitor", "Progress and workload operations traverse the decorated tree.");
        ProgressReportVisitor progress;
        WorkloadVisitor workload;
        instance.getRoot()->accept(progress);
        instance.getRoot()->accept(workload);
        for (const std::string& line : progress.getReport()) std::cout << "  " << line << '\n';
        for (const std::string& line : workload.getSummary()) std::cout << "  " << line << '\n';
        require(completed == std::vector<std::string>({"diagnose", "repair", "verify", "repair", "verify", "close"}),
                "Failed verification loops back to repair; passed verification closes the case");
        require(progress.isComplete(), "All four task objects are completed");
        require(engine.getHistory(instanceId).size() == 18, "History retains eighteen successful lifecycle commands");
        require(audit.getLog().size() == 28 && notifications.getNotifications().size() == 28,
                "Both observers received all twenty-eight task state changes");
        require(engine.getAuditTrail(instanceId).size() == 28, "Facade retrieves this case's audit trail");
        stageGuard->setState("COMPLETED");
        instance.setState("COMPLETED");
        instance.print();
        std::cout << "\nPatterns demonstrated: Builder, Composite, Factory, Strategy, Facade,\n"
                     "Decorator (Security + AllowedStates + Logging), Iterator, Command, State, Observer,\n"
                     "Chain of Responsibility, Memento and Prototype, Visitor.\n"
                     "SCENARIO 1 PASSED\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "SCENARIO 1 FAILED: " << error.what() << '\n';
        return 1;
    }
}
