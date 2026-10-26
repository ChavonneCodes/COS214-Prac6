// Scenario 1: urgent project review followed by manager approval.
// Build: make Scenario1     Run: ./Scenario1 (PowerShell: .\Scenario1.exe)
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
    require(engine.submit(command.release()), "Command executed for " + item.getId());
    require(item.getCurrentState()->getName() == expectedState,
            item.getId() + " reached State object " + expectedState);
}

int main() {
    try {
        std::cout << "=== SCENARIO 1: PROJECT APPROVAL ===\n";
        std::cout << "\n1. Build the workflow definition (Builder + Composite)\n";
        WorkflowBlueprint blueprint;
        blueprint.id = "project-approval";
        blueprint.name = "Urgent project approval";
        blueprint.roles.push_back(RoleDefinition("reviewer", "Reviewer", 1));
        blueprint.roles.push_back(RoleDefinition("manager", "Manager", 2));
        blueprint.activities.push_back(ActivityDefinition("documents", "Check documents", "reviewer"));
        blueprint.activities.push_back(ActivityDefinition("technical", "Urgent technical review", "reviewer"));
        blueprint.activities.push_back(ActivityDefinition("approve", "Approve project", "manager"));
        blueprint.transitions.push_back(TransitionDefinition("documents", "approve", "always"));
        blueprint.transitions.push_back(TransitionDefinition("technical", "approve", "always"));
        blueprint.hasRootTree = true;
        blueprint.rootTreeData.type = NodeType::STAGE;
        blueprint.rootTreeData.id = "reviews";
        blueprint.rootTreeData.name = "Project review stage";
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

        std::cout << "\n2. Create people, observers and decorated runtime tasks\n";
        Participant visitor("p0", "Visitor", {"guest"});
        Participant reviewer("p1", "Alice", {"reviewer"}, 1);
        Participant manager("p2", "Bob", {"manager"}, 2);
        std::vector<Participant*> candidates = {&visitor, &reviewer, &manager};
        RoleBasedAssignment assignment;
        AuditObserver audit;
        NotificationObserver notifications("STATE_CHANGED");
        EventBus bus;
        bus.attach(&audit);
        bus.attach(&notifications);
        HistoryManager history;
        WorkflowEngine engine(nullptr, &bus, &history, &audit);

        auto* stageDefinition = dynamic_cast<StageDefinition*>(definition.getRoot());
        require(stageDefinition != nullptr, "Builder produced a stage");
        std::unique_ptr<RuntimeComponent> root(new PriorityRuntimeDecorator(
            new SecurityRuntimeDecorator(new StageInstance("project-001", stageDefinition),
                                         "manager", &manager), 0));
        // These maps borrow pointers; root owns the tasks and decorators.
        std::map<std::string, WorkItem*> items;
        std::map<std::string, SecurityRuntimeDecorator*> guards;
        for (WorkflowComponent* node : definition.getRoot()->getChildren()) {
            // Defer assignment to AssignCommand so the State pattern is exercised.
            std::unique_ptr<RuntimeComponent> created(
                InstanceFactory::instantiateTree("project-001", node, nullptr, candidates));
            auto* item = dynamic_cast<WorkItem*>(created.get());
            require(item != nullptr, "Factory created " + node->getId());
            item->setEventBus(&bus);
            Participant* actor = assignment.assign(*item, candidates);
            require(actor != nullptr && actor->hasRole(item->getRequiredRoleId()),
                    "Strategy found an eligible participant for " + item->getId());
            // Approval has the highest priority but still waits for both reviews.
            int priority = item->getId() == "approve" ? 100 :
                           item->getId() == "technical" ? 10 : 1;
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

        std::cout << "\n3. Reject an unauthorized attempt (Security Decorator)\n";
        auto* technicalGuard = guards.at("technical");
        technicalGuard->setActor(&visitor);
        const auto eventsBefore = audit.getLog().size();
        const auto stateBefore = technicalGuard->getState();
        bool denied = false;
        try {
            technicalGuard->setState("SUCCESS");
        } catch (const std::runtime_error& error) {
            denied = true;
            std::cout << "Expected rejection: " << error.what() << '\n';
        }
        require(denied && technicalGuard->getState() == stateBefore &&
                audit.getLog().size() == eventsBefore,
                "Denied attempt changed neither task state nor observer log");
        technicalGuard->setActor(&reviewer);

        std::cout << "\n4. Process eligible tasks in priority order\n";
        SequentialRouting routing;
        RoleApprovalHandler reviewerHandler("reviewer");
        RoleApprovalHandler managerHandler("manager");
        reviewerHandler.setNext(&managerHandler);
        std::vector<std::string> completed;
        bool approvalRouted = false;
        while (completed.size() < items.size()) {
            bool progressed = false;
            // A traversal consumer uses virtual getChildren(); no iterator class
            // for these runtime components exists yet.
            for (RuntimeComponent* component : root->getChildren()) {
                WorkItem& item = *items.at(component->getId());
                SecurityRuntimeDecorator& guard = *guards.at(component->getId());
                if (item.getState() == "Completed") continue;
                if (item.getId() == "approve") {
                    if (!approvalRouted || items.at("documents")->getState() != "Completed" ||
                        items.at("technical")->getState() != "Completed") {
                        std::cout << "Approval waits for both reviews despite priority 100.\n";
                        continue;
                    }
                    Request request("approve", item.getId(), "project-001", "manager", "Reviews passed");
                    require(reviewerHandler.authorise(request),
                            "Approval chain passes request to the manager handler");
                }
                std::cout << "Processing " << item.getId() << " (priority "
                          << component->getPriority() << ")\n";
                require(guard.canModify() && item.makeAvailable(), "Task becomes available");
                Participant* chosen = assignment.assign(item, candidates);
                require(chosen != nullptr, "Assignment strategy selects " + item.getId());
                submit(engine, guard, item,
                       std::unique_ptr<WorkflowAction>(new AssignCommand(&item, chosen)), "Assigned");
                submit(engine, guard, item,
                       std::unique_ptr<WorkflowAction>(new StartCommand(&item)), "In_Progress");

                if (item.getId() == "technical") {
                    std::cout << "Save review notes, make a mistake, then restore (Memento).\n";
                    Data notes("Technical checks passed");
                    // Memento borrows its state pointer; keep that snapshot alive.
                    InProgressState savedState;
                    WorkItemMemento snapshot(item.getId(), "project-001/technical", notes, &savedState);
                    notes.setValue("Accidentally overwritten notes");
                    notes = snapshot.getDataSnapshot();
                    require(notes.getValue() == "Technical checks passed" &&
                            snapshot.getStateKind()->getName() == item.getCurrentState()->getName(),
                            "Snapshot restores notes and records the running state");
                }
                submit(engine, guard, item,
                       std::unique_ptr<WorkflowAction>(new CompleteCommand(&item)), "Completed");
                completed.push_back(item.getId());
                progressed = true;
                const ActivityDefinition* activity = findActivity(definition, item.getId());
                require(activity != nullptr, "Routing can find completed activity");
                for (const auto* next : routing.next(*activity, definition, "passed")) {
                    std::cout << "Routing proposes next task: " << next->getId() << '\n';
                    if (next->getId() == "approve") approvalRouted = true;
                }
            }
            require(progressed, "Workflow made progress without bypassing dependencies");
        }

        std::cout << "\n5. Verify the completed workflow and its audit trail\n";
        require(completed == std::vector<std::string>({"technical", "documents", "approve"}),
                "Urgent review first, normal review second, approval last");
        require(items.at("technical")->getAssignedParticipant() == &reviewer &&
                items.at("approve")->getAssignedParticipant() == &manager,
                "Correct people performed reviews and approval");
        require(history.getAll().size() == 9, "History records all nine successful commands");
        require(audit.getLog().size() == 12 && notifications.getNotifications().size() == 12,
                "Both observers received all twelve task state changes");
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
