// Scenario 1: restore an unavailable campus service before routine support work.
// Compile this file with the project's non-main .cpp files; run .\scenario1.exe.
#include "ConcreteWorkflowBuilder.h"
#include "WorkflowDirector.h"
#include "InstanceFactory.h"
#include "StageInstance.h"
#include "WorkItem.h"
#include "RoleBasedAssignment.h"
#include "PriorityRuntimeDecorator.h"
#include "SecurityRuntimeDecorator.h"
#include "AuditObserver.h"
#include "NotificationObserver.h"
#include "WorkflowEngine.h"
#include "AssignCommand.h"
#include "StartCommand.h"
#include "CompleteCommand.h"
#include "RoleApprovalHandler.h"
#include "SequentialRouting.h"
#include "RoutingHelper.h"
#include "WorkItemMemento.h"
#include "InProgressState.h"
#include <map>
#include <set>
#include <memory>
#include <stdexcept>

static void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
    std::cout << "[PASS] " << message << '\n';
}

// The current commands accept WorkItem*, so this scenario explicitly checks
// security before submitting them. This is scenario glue, not engine enforcement.
// The engine owns submitted commands and records successful ones in history.
static void submit(WorkflowEngine& engine,
                   SecurityRuntimeDecorator& guard, WorkItem& item,
                   std::unique_ptr<WorkflowAction> command, const std::string& expectedState) {
    require(guard.canModify(), "Actor may modify " + item.getId());
    const std::string before = item.getState();
    std::cout << "  [COMMAND] " << command->getDescription() << '\n';
    require(engine.submit(command.release()), "Command executed for " + item.getId());
    std::cout << "  [STATE] " << item.getId() << ": " << before
              << " -> " << item.getState() << '\n';
    require(item.getCurrentState()->getName() == expectedState,
            item.getId() + " reached State object " + expectedState);
}

int main() {
    try {
        std::cout << "=== SCENARIO 1: CAMPUS SERVICE OUTAGE ===\n";
        std::cout << "Students cannot access the learning portal. A printer ticket is also waiting.\n"
                     "Goal: diagnose -> repair -> verify the portal, then handle the printer.\n"
                     "Priority chooses between ready tasks; it never bypasses dependencies.\n";
        std::cout << "\n1. Build the workflow definition (Builder + Composite)\n";
        WorkflowBlueprint blueprint;
        blueprint.id = "service-recovery";
        blueprint.name = "Restore the campus learning portal";
        blueprint.roles.push_back(RoleDefinition("technician", "Support technician", 1));
        blueprint.roles.push_back(RoleDefinition("administrator", "Service administrator", 2));
        // Insert routine work first to show that priority changes execution order.
        blueprint.activities.push_back(ActivityDefinition("printer", "Resolve a routine printer ticket", "technician"));
        blueprint.activities.push_back(ActivityDefinition("diagnose", "Diagnose the portal outage", "technician"));
        blueprint.activities.push_back(ActivityDefinition("repair", "Restore the portal configuration", "administrator"));
        blueprint.activities.push_back(ActivityDefinition("verify", "Verify that students can log in", "technician"));
        blueprint.transitions.push_back(TransitionDefinition("diagnose", "repair", "always"));
        blueprint.transitions.push_back(TransitionDefinition("repair", "verify", "always"));
        blueprint.hasRootTree = true;
        blueprint.rootTreeData.type = NodeType::STAGE;
        blueprint.rootTreeData.id = "support";
        blueprint.rootTreeData.name = "Campus IT service desk";
        for (const auto& activity : blueprint.activities) {
            WorkflowNodeData node;
            node.type = NodeType::ACTIVITY;
            node.id = activity.getId();
            node.name = activity.getName();
            node.roleId = activity.getRequiredRoleId();
            blueprint.rootTreeData.children.push_back(node);
        }
        ConcreteWorkflowBuilder builder;
        WorkflowDirector director;
        director.construct(builder, blueprint);
        WorkflowDefinition definition = builder.build();
        definition.getRoot()->print();
        std::cout << "[ROUTES] diagnose -> repair -> verify\n"
                     "[INDEPENDENT] printer has no dependency, but has the lowest priority.\n";

        std::cout << "\n2. Create people, observers and decorated runtime tasks\n";
        Participant visitor("p0", "Student", {"guest"});
        Participant technician("p1", "Alice", {"technician"}, 1);
        Participant administrator("p2", "Bob", {"administrator"}, 2);
        std::vector<Participant*> candidates = {&visitor, &technician, &administrator};
        RoleBasedAssignment assignment;
        AuditObserver audit;
        NotificationObserver notifications("STATE_CHANGED");
        EventBus bus;
        bus.attach(&audit);
        bus.attach(&notifications);
        std::cout << "[PEOPLE] Student: guest | Alice: technician | Bob: administrator\n"
                     "[OBSERVERS] Audit and notifications listen for STATE_CHANGED events.\n";
        HistoryManager history;
        WorkflowEngine engine(nullptr, &bus, &history, &audit);

        auto* stageDefinition = dynamic_cast<StageDefinition*>(definition.getRoot());
        require(stageDefinition != nullptr, "Builder produced a stage");
        std::unique_ptr<RuntimeComponent> root(new PriorityRuntimeDecorator(
            new SecurityRuntimeDecorator(new StageInstance("outage-001", stageDefinition),
                                         "administrator", &administrator), 0));
        // These maps borrow pointers; root owns the tasks and decorators.
        std::map<std::string, WorkItem*> items;
        std::map<std::string, SecurityRuntimeDecorator*> guards;
        for (WorkflowComponent* node : definition.getRoot()->getChildren()) {
            // Defer assignment to AssignCommand so the State pattern is exercised.
            std::unique_ptr<RuntimeComponent> created(
                InstanceFactory::instantiateTree("outage-001", node, nullptr, candidates));
            auto* item = dynamic_cast<WorkItem*>(created.get());
            require(item != nullptr, "Factory created " + node->getId());
            item->setEventBus(&bus);
            Participant* actor = assignment.assign(*item, candidates);
            require(actor != nullptr && actor->hasRole(item->getRequiredRoleId()),
                    "Strategy found an eligible participant for " + item->getId());
            // Repair is urgent but cannot run before diagnosis unlocks it.
            int priority = item->getId() == "repair" ? 100 :
                           item->getId() == "verify" ? 90 :
                           item->getId() == "diagnose" ? 80 : 1;
            std::cout << "[TASK] " << item->getId() << " | priority=" << priority
                      << " | required role=" << item->getRequiredRoleId()
                      << " | eligible actor=" << actor->getName()
                      << " | initial state=" << item->getState() << '\n';
            auto* guard = new SecurityRuntimeDecorator(
                new PriorityRuntimeDecorator(created.release(), priority),
                item->getRequiredRoleId(), actor);
            std::unique_ptr<RuntimeComponent> decorated(guard);
            root->addChild(decorated.get());
            decorated.release();
            items[item->getId()] = item;
            guards[item->getId()] = guard;
        }
        root->setState("RUNNING");
        std::cout << "[PRIORITY ORDER]";
        for (RuntimeComponent* child : root->getChildren()) {
            std::cout << ' ' << child->getId() << '(' << child->getPriority() << ')';
        }
        std::cout << "\nThis differs from insertion order: printer, diagnose, repair, verify.\n";

        std::cout << "\n3. Reject an unauthorized attempt (Security Decorator)\n";
        auto* repairGuard = guards.at("repair");
        repairGuard->setActor(&visitor);
        const auto eventsBefore = audit.getLog().size();
        const auto stateBefore = repairGuard->getState();
        bool denied = false;
        std::cout << "[ATTEMPT] Student tries to mark the administrator-only repair as complete.\n";
        try {
            repairGuard->setState("SUCCESS");
        } catch (const std::runtime_error& error) {
            denied = true;
            std::cout << "Expected rejection: " << error.what() << '\n';
        }
        require(denied && repairGuard->getState() == stateBefore &&
                audit.getLog().size() == eventsBefore,
                "Denied attempt changed neither task state nor observer log");
        repairGuard->setActor(&administrator);
        std::cout << "[SECURITY] Repair remains " << repairGuard->getState()
                  << "; audit entries remain " << audit.getLog().size()
                  << ". Bob is restored as the acting administrator.\n";

        std::cout << "\n4. Process eligible tasks in priority order\n";
        SequentialRouting routing;
        RoleApprovalHandler technicianHandler("technician");
        RoleApprovalHandler administratorHandler("administrator");
        technicianHandler.setNext(&administratorHandler);
        std::vector<std::string> completed;
        std::set<std::string> ready = {"diagnose", "printer"};
        while (completed.size() < items.size()) {
            std::cout << "\n--- Scheduling decision " << completed.size() + 1 << " ---\n";
            for (RuntimeComponent* candidate : root->getChildren()) {
                std::cout << "  " << candidate->getId() << " | priority="
                          << candidate->getPriority() << " | state=" << candidate->getState()
                          << " | " << (candidate->getState() == "Completed" ? "finished" :
                              ready.count(candidate->getId()) ? "ready" : "waiting for dependency") << '\n';
            }
            bool progressed = false;
            // A traversal consumer uses virtual getChildren(); no iterator class
            // for these runtime components exists yet.
            for (RuntimeComponent* component : root->getChildren()) {
                WorkItem& item = *items.at(component->getId());
                SecurityRuntimeDecorator& guard = *guards.at(component->getId());
                if (item.getState() == "Completed") continue;
                if (ready.count(item.getId()) == 0) {
                    std::cout << item.getId() << " waits for its preceding task despite its priority.\n";
                    continue;
                }
                if (item.getId() == "repair") {
                    std::cout << "[CHAIN] Repair needs administrator authority. The technician\n"
                                 "        handler forwards the request to the administrator handler.\n";
                    Request request("repair", item.getId(), "outage-001", "administrator", "Diagnosis confirms incorrect configuration");
                    require(technicianHandler.authorise(request),
                            "Repair request reaches the administrator through the responsibility chain");
                }
                std::cout << "Processing " << item.getId() << " (priority "
                          << component->getPriority() << ")\n";
                require(guard.canModify() && item.makeAvailable(), "Task becomes available");
                Participant* chosen = assignment.assign(item, candidates);
                require(chosen != nullptr, "Assignment strategy selects " + item.getId());
                std::cout << "[ASSIGNMENT] " << chosen->getName() << " will perform "
                          << item.getId() << " (role: " << item.getRequiredRoleId() << ").\n";
                submit(engine, guard, item,
                       std::unique_ptr<WorkflowAction>(new AssignCommand(&item, chosen)), "Assigned");
                submit(engine, guard, item,
                       std::unique_ptr<WorkflowAction>(new StartCommand(&item)), "In_Progress");

                if (item.getId() == "repair") {
                    std::cout << "Recover the saved repair plan after an accidental edit (Memento).\n";
                    Data notes("Restore the last working portal configuration");
                    // Memento borrows its state pointer; keep that snapshot alive.
                    InProgressState savedState;
                    WorkItemMemento snapshot(item.getId(), "outage-001/repair", notes, &savedState);
                    std::cout << "[MEMENTO] Saved: " << notes.getValue() << '\n';
                    notes.setValue("Accidentally replaced repair instructions");
                    std::cout << "[MEMENTO] After accidental edit: " << notes.getValue() << '\n';
                    notes = snapshot.getDataSnapshot();
                    std::cout << "[MEMENTO] Restored: " << notes.getValue() << '\n';
                    require(notes.getValue() == "Restore the last working portal configuration" &&
                            snapshot.getStateKind()->getName() == item.getCurrentState()->getName(),
                            "Snapshot restores the repair plan and records the running state");
                }
                submit(engine, guard, item,
                       std::unique_ptr<WorkflowAction>(new CompleteCommand(&item)), "Completed");
                completed.push_back(item.getId());
                std::cout << "[OBSERVERS] Audit entries=" << audit.getLog().size()
                          << ", notifications=" << notifications.getNotifications().size() << '\n';
                if (item.getId() == "verify") {
                    std::cout << "[SERVICE] Portal verification is complete. Routine printer support can now follow.\n";
                }
                progressed = true;
                const ActivityDefinition* activity = findActivity(definition, item.getId());
                require(activity != nullptr, "Routing can find completed activity");
                for (const auto* next : routing.next(*activity, definition, "passed")) {
                    std::cout << "[ROUTING] " << item.getId() << " completed; "
                              << next->getId() << " is now ready.\n";
                    ready.insert(next->getId());
                }
                // Reconsider priorities after each completion: newly unlocked
                // outage work must run before the routine printer ticket.
                break;
            }
            require(progressed, "Workflow made progress without bypassing dependencies");
        }

        std::cout << "\n5. Verify the completed workflow and its audit trail\n";
        std::cout << "[COMPLETION ORDER] ";
        for (size_t i = 0; i < completed.size(); ++i) {
            if (i != 0) std::cout << " -> ";
            std::cout << completed[i];
        }
        std::cout << '\n';
        require(completed == std::vector<std::string>({"diagnose", "repair", "verify", "printer"}),
                "Diagnose, repair and verify the outage before routine printer support");
        require(items.at("diagnose")->getAssignedParticipant() == &technician &&
                items.at("repair")->getAssignedParticipant() == &administrator &&
                items.at("verify")->getAssignedParticipant() == &technician,
                "Technician diagnoses and verifies; administrator performs the repair");
        require(history.getAll().size() == 12, "History records all twelve successful commands");
        require(audit.getLog().size() == 16 && notifications.getNotifications().size() == 16,
                "Both observers received all sixteen task state changes");
        root->setState("SUCCESS");
        root->print();
        audit.printLog();
        std::cout << "\nSCENARIO 1 PASSED\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "SCENARIO 1 FAILED: " << error.what() << '\n';
        return 1;
    }
        
}
