/**
 * Campus portal recovery: interactive demonstration using the existing engine.
 * Five tasks with Security, Password, AllowedStates and Logging decorators.
 * The presenter drives participant actions; this scenario still coordinates routing.
 */
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>
#include <sstream>
#include "RejectCommand.h"
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
#include "SequentialRouting.h"
#include "ConditionalRouting.h"
#include "RoutingHelper.h"
#include "ValidationApprovalHandler.h"
#include "RoleApprovalHandler.h"
#include "AssignCommand.h"
#include "StartCommand.h"
#include "CompleteCommand.h"
#include "EscalateCommand.h"
#include "PasswordRuntimeDecorator.h"
#include "DependencyRuntimeDecorator.h"
#include "ProgressReportVisitor.h"
#include "WorkloadVisitor.h"
#include "NotificationObserver.h"
#include "WorkloadBasedAssignment.h"

namespace {
/**
 * @brief Reversible stage metadata update through the existing decorator chain.
 * Work-item lifecycle changes use Felicity's existing commands instead.
 * The stage and its owning instance must outlive this command (engine-owned).
 */
class StageStateCommand : public WorkflowAction {
    RuntimeComponent& stage;
    WorkflowInstance& instance;
    std::string requested, previousStage, previousInstance;
    bool executed = false;
public:
    StageStateCommand(RuntimeComponent& stage, WorkflowInstance& instance,
                      const std::string& requested)
        : stage(stage), instance(instance), requested(requested) {}
    bool execute() override {
        previousStage = stage.getState();
        previousInstance = instance.getState();
        // Security and AllowedStates may throw before either object changes.
        stage.setState(requested);
        instance.setState(requested);
        executed = true;
        return true;
    }
    void undo() override {
        if (!executed) return;
        stage.setState(previousStage);
        instance.setState(previousInstance);
        executed = false;
    }
    std::string getDescription() override { return "Stage " + stage.getId() + " -> " + requested; }
    std::string getInstanceId() override { return instance.getId(); }
};

/** Scenario adapter: enforces decorators for participant actions and restores
 * state, assignee and workload together. System activation passes no guard. */
class TaskCommand : public WorkflowAction {
    WorkItem& item;
    RuntimeComponent* guard;
    std::unique_ptr<WorkflowAction> action;
    std::unique_ptr<WorkItemMemento> before;
    Participant* previous = nullptr;
    static bool open(const std::string& state) {
        return state == "Assigned" || state == "In_Progress" || state == "Rejected";
    }
    void release() {
        if (open(item.getState()) && item.getAssignedParticipant())
            item.getAssignedParticipant()->closeOpenItem();
    }
    void acquire() {
        if (open(item.getState()) && item.getAssignedParticipant())
            item.getAssignedParticipant()->addOpenItem();
    }
public:
    TaskCommand(WorkItem& item, WorkflowAction* action, RuntimeComponent* guard = nullptr)
        : item(item), guard(guard), action(action) {}
    bool execute() override {
        auto operation = [&]() {
            before.reset(new WorkItemMemento(item.createMemento()));
            previous = item.getAssignedParticipant();
            release();
            bool ok = action->execute();
            if (ok && item.getState() == "Escalated") item.setParticipant(nullptr);
            acquire();
            return ok;
        };
        return guard ? guard->execute(operation) : operation();
    }
    void undo() override {
        if (!before) return;
        release();
        item.restoreMemento(before.get());
        item.setParticipant(previous);
        acquire();
        std::cout << "Task action undone: " << item.getId() << '\n';
    }
    std::string getDescription() override { return action->getDescription(); }
    std::string getInstanceId() override { return item.getInstanceId(); }
};
class makeAvailableCommand : public WorkflowAction {
    WorkItem* item;
public:
    explicit makeAvailableCommand(WorkItem* item) : item(item) {}
    bool execute() override { return item->makeAvailable(); }
    void undo() override {} // TaskCommand owns the snapshot.
    std::string getDescription() override { return "Make available: " + item->getId(); }
    std::string getInstanceId() override { return item->getInstanceId(); }
};

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error("Check failed: " + message);
}
void section(const std::string& title) {
    std::cout << "\n============================================================\n"
              << title << "\n============================================================\n";
}
void pattern(const std::string& name, const std::string& purpose) {
    std::cout << "[Pattern: " << name << "] " << purpose << '\n';
}

// Known demo keys are separate from roles and from a task's temporary unlock grant.
using ParticipantKeys = std::map<std::string, std::map<std::string, std::string>>;

void printParticipant(const Participant& participant, const ParticipantKeys& keys) {
    std::cout << participant.getName() << " | roles: ";
    const auto& roles = participant.getRoleIds();
    if (roles.empty()) std::cout << "none";
    for (size_t i = 0; i < roles.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << roles[i];
    }
    std::cout << " | " << participant.getOpenItems() << " unfinished assignments | keys: ";
    const auto found = keys.find(participant.getParticipantId());
    if (found == keys.end() || found->second.empty()) std::cout << "none";
    else {
        bool first = true;
        for (const auto& key : found->second) {
            if (!first) std::cout << ", ";
            std::cout << key.second;
            first = false;
        }
    }
}

void printParticipants(const std::vector<Participant*>& participants, const ParticipantKeys& keys) {
    section("PARTICIPANTS AND ROLES");
    for (const Participant* participant : participants) {
        printParticipant(*participant, keys);
        std::cout << '\n';
    }
    std::cout << "Workload counts unfinished assignments, including other incidents.\n"
              << "Sam starts with two existing assignments outside this incident.\n"
              << "Assignment strategy chooses the least-busy participant with the required role.\n"
              << "Displayed keys are known demo values; enter the required key for each protected task.\n";
}

WorkflowBlueprint makeBlueprint() {
    WorkflowBlueprint plan;
    plan.id = "campus-recovery";
    plan.name = "Restore the campus learning portal";
    plan.roles.push_back(RoleDefinition("technician", "Support technician", 1));
    plan.roles.push_back(RoleDefinition("administrator", "Service administrator", 2));
    plan.activities = {
        ActivityDefinition("triage", "Record and triage the incident", "technician"),
        ActivityDefinition("diagnose", "Diagnose the outage", "technician"),
        ActivityDefinition("repair", "Restore the portal configuration", "administrator"),
        ActivityDefinition("verify", "Test student access", "technician"),
        ActivityDefinition("close", "Close the incident", "administrator")
    };
    for (size_t i = 1; i < plan.activities.size(); ++i)
        plan.transitions.push_back(TransitionDefinition(
            plan.activities[i-1].getId(), plan.activities[i].getId(), "always"));
    // Verification overrides sequential routing: passed -> close, failed -> repair.
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


// Assignment can change the participant without restarting work already in progress.
class AssignParticipantCommand : public WorkflowAction {
    WorkItem& item;
    Participant* participant;
public:
    AssignParticipantCommand(WorkItem& item, Participant* participant)
        : item(item), participant(participant) {}
    bool execute() override {
        if (!participant) throw std::runtime_error("no participant selected");
        const std::string state = item.getState();
        if (state == "Available" || state == "Escalated") return item.assign(participant);
        if (state == "Assigned" || state == "In_Progress" || state == "Rejected") {
            item.setParticipant(participant);
            return true;
        }
        throw std::runtime_error("task is " + state + "; participant changes require active work");
    }
    void undo() override {} // TaskCommand restores the state, participant and workload.
    std::string getDescription() override {
        return "Assign participant " + participant->getName() + " for " + item.getId();
    }
    std::string getInstanceId() override { return item.getInstanceId(); }
};

bool submit(WorkflowEngine& engine, RuntimeComponent& guard, WorkItem& item,
            std::unique_ptr<WorkflowAction> command, const std::string& action) {
    const std::string before = item.getState();
    try {
        if (engine.submit(new TaskCommand(item, command.release(), &guard))) {
            std::cout << action << " succeeded: " << item.getName() << " | " << before
                      << " -> " << item.getState() << '\n';
            return true;
        }
        std::cout << "Could not " << action << " '" << item.getName()
                  << "': its current state (" << before << ") does not allow this action.\n";
    } catch (const std::exception& error) {
        std::cout << "Could not " << action << " '" << item.getName() << "': " << error.what() << ".\n";
    }
    return false;
}

struct InputClosed {};
std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) throw InputClosed();
    return line;
}
int choose(const std::string& prompt, int minimum, int maximum) {
    for (;;) {
        std::istringstream input(readLine(prompt));
        int value;
        if (input >> value) {
            input >> std::ws;
            if (input.eof() && value >= minimum && value <= maximum) return value;
        }
        std::cout << "Please enter a whole number from " << minimum << " to " << maximum << ".\n";
    }
}
void guide() {
    section("DEMO GUIDE");
    std::cout << "Pick any of the five tasks. Its metadata names prerequisites, role and key.\n"
        << "Progress uses WorkloadBased assignment, then starts and completes the task.\n"
        << "Each successful step is separate: press Progress again for the next step.\n"
        << "Assign participant lists everyone, including people with an unsuitable role.\n"
        << "Blocked dependencies, wrong roles and missing keys are checked by decorators.\n"
        << "Repair needs an access key. Demo value: portal-key.\n"
        << "For repair, Progress selects Bob; enter the key with action 2 to finish assignment.\n"
        << "Known keys stay with participants; task unlocks clear when participants change or work rolls back.\n"
        << "Verification failure rolls repair and verification back for another attempt.\n"
        << "Closure requires completed verification with a Passed result.\n"
        << "Use 0 to go back, inspect participants or reports, or undo the last successful action.\n\n"
        << "Builder/Composite/Prototype define the case; Factory creates its runtime tree.\n"
        << "State/Command/Memento manage work and undo; Decorator enforces access and prerequisites.\n"
        << "Routing Strategy controls progression; Observer keeps audit events after rollback.\n"
        << "Approval uses Chain of Responsibility; Iterator/Visitor inspect the runtime tree.\n"
        << "This is an in-memory participant role-play; the presenter acts for the selected participant.\n";
}
struct Navigation {
    std::string current = "triage";
    std::vector<std::string> visits;
    std::map<std::string, size_t> checkpoints;
    bool verificationPassed = false;
};
struct UndoPoint {
    size_t historySize;
    Navigation navigation;
};
} // namespace

int main() {
    try {
        section("TIVIDY | CAMPUS PORTAL RECOVERY");
        for (;;) {
            std::cout << "1. Start an incident\n2. Demo guide\n0. Exit\n";
            int selection = choose("Choice: ", 0, 2);
            if (!selection) return 0;
            if (selection == 1) break;
            guide();
        }
        std::string description = readLine("Incident summary (Enter: students cannot log in): ");
        if (description.empty()) description = "Students cannot log into the learning portal";
        ConcreteWorkflowBuilder builder;
        WorkflowDirector director;
        WorkflowBlueprint blueprint = makeBlueprint();
        director.construct(builder, blueprint);
        InMemoryDefinitionRepository repository;
        repository.save(new WorkflowDefinition(builder.build()));
        WorkflowDefinition* definition = repository.load("campus-recovery", 1);
        require(definition != nullptr, "Definition v1 is stored");
        repository.save(definition->cloneWithVersion(2));
        require(repository.load("campus-recovery", 2)->getRoot() != definition->getRoot(),
                "Version 2 owns an independent definition tree");

        Participant student("p0", "Student", {"guest"});
        Participant busy("p1", "Sam", {"technician"}, 1);
        Participant alice("p2", "Alice", {"technician"}, 1);
        Participant bob("p3", "Bob", {"administrator"}, 2);
        busy.addOpenItem(); busy.addOpenItem();
        std::vector<Participant*> staff = {&student, &busy, &alice, &bob};
        const std::string recoveryKey = "portal-key";
        WorkloadBasedAssignment assignment;
        ParticipantKeys participantKeys = {
            {bob.getParticipantId(), {{"portal recovery key", recoveryKey}}}
        };
        EventBus bus;
        AuditObserver audit;
        NotificationObserver notifications("STATE_CHANGED");
        bus.attach(&audit); bus.attach(&notifications);
        HistoryManager history;
        Navigation navigation;
        std::vector<UndoPoint> undoPoints;
        const std::string instanceId = "portal-incident-001";
        std::map<std::string, WorkItem*> items;
        std::map<std::string, DependencyRuntimeDecorator*> guards;
        std::map<std::string, PasswordRuntimeDecorator*> passwords;
        std::map<std::string, std::string> prerequisites;
        for (size_t i = 1; i < blueprint.activities.size(); ++i)
            prerequisites[blueprint.activities[i].getId()] = blueprint.activities[i - 1].getId();

        auto dependencyReason = [&](const std::string& id) -> std::string {
            if (prerequisites.count(id)) {
                WorkItem& previous = *items.at(prerequisites.at(id));
                if (previous.getState() != "Completed")
                    return "dependency not met: complete '" + previous.getName() + "' (" +
                           previous.getId() + ") first; its state is " + previous.getState();
            }
            if (id == "close" && !navigation.verificationPassed)
                return "dependency not met: 'Test student access' must have a Passed result";
            return "";
        };
        auto* stageDefinition = dynamic_cast<StageDefinition*>(definition->getRoot());
        require(stageDefinition != nullptr, "Definition root is a stage");
        std::unique_ptr<SecurityRuntimeDecorator> root(new SecurityRuntimeDecorator(
            new AllowedStatesRuntimeDecorator(new LoggingRuntimeDecorator(
                new StageInstance(instanceId, stageDefinition)), {"IDLE", "RUNNING", "COMPLETED"}),
            "administrator", &bob));
        for (WorkflowComponent* node : definition->getRoot()->getChildren()) {
            std::unique_ptr<RuntimeComponent> created(
                InstanceFactory::instantiateTree(instanceId, node, nullptr, staff));
            auto* item = dynamic_cast<WorkItem*>(created.get());
            require(item != nullptr, "Factory created a work item");
            item->setEventBus(&bus);
            const std::string id = item->getId();
            RuntimeComponent* decorated = created.release();
            if (id == "repair") {
                auto* password = new PasswordRuntimeDecorator(decorated, recoveryKey, "portal recovery key");
                passwords[id] = password;
                decorated = password;
            }
            decorated = new SecurityRuntimeDecorator(decorated, item->getRequiredRoleId());
            const std::string dependency = prerequisites.count(id)
                ? "complete " + findNode(*definition, prerequisites.at(id))->getName() +
                    (id == "close" ? " with Passed result" : "")
                : "none (entry task)";
            std::unique_ptr<DependencyRuntimeDecorator> guard(new DependencyRuntimeDecorator(
                decorated, dependency, [&, id]() { return dependencyReason(id); }));
            root->addChild(guard.get());
            items[id] = item;
            guards[id] = guard.release();
        }
        SecurityRuntimeDecorator* stageGuard = root.get();
        WorkflowInstance instance(instanceId, definition->getDefinitionId(), 1, root.release(), &bus);
        instance.getData().setValue(description);
        // Commands are destroyed before the locally owned instance and participants.
        WorkflowEngine engine(&repository, &bus, &history, &audit);
        pattern("Facade + Command", "Task actions pass through decorators and capture rollback snapshots.");
        require(engine.submit(new StageStateCommand(*stageGuard, instance, "RUNNING")), "Cannot start incident");
        bus.notify(WorkFlowEvent("INCIDENT_STARTED", instanceId, "-", description));
        SequentialRouting sequential;
        ConditionalRouting verification("passed", "close", "repair");
        ValidationApprovalHandler validation;
        RoleApprovalHandler technicianApproval("technician"), administratorApproval("administrator");
        validation.setNext(&technicianApproval);
        technicianApproval.setNext(&administratorApproval);

        auto resetAccess = [&]() {
            for (const auto& entry : guards) entry.second->setActor(items.at(entry.first)->getAssignedParticipant());
            for (const auto& entry : passwords) entry.second->lock();
        };
        auto rollback = [&](size_t checkpoint) {
            while (history.getAll().size() > checkpoint) engine.undoLast();
            resetAccess();
        };
        auto offer = [&](const std::string& id) {
            WorkItem& item = *items.at(id);
            require(item.getState() == "Created", "Reached task must be Created before activation");
            navigation.checkpoints[id] = history.getAll().size();
            // Workflow activation only offers work. A participant must be chosen explicitly.
            require(engine.submit(new TaskCommand(item, new makeAvailableCommand(&item))), "Cannot offer task");
            guards.at(id)->setActor(nullptr);
            if (passwords.count(id)) passwords.at(id)->lock();
            std::cout << "Ready to assign: " << item.getName() << ".\n";
        };
        auto undo = [&]() {
            if (undoPoints.empty()) {
                std::cout << "Nothing to undo since case start or the latest verification retry checkpoint.\n";
                return;
            }
            UndoPoint point = undoPoints.back(); undoPoints.pop_back();
            rollback(point.historySize);
            navigation = point.navigation;
            bus.notify(WorkFlowEvent("UNDO", instanceId, navigation.current, "Presenter undid the last action"));
            std::cout << "Last action undone, including any successor activation. Audit events retained.\n";
        };
        auto printTask = [&](const std::string& id, size_t number) {
            WorkItem& item = *items.at(id);
            if (number) std::cout << number << ". ";
            std::cout << guards.at(id)->getName() << '\n'
                      << "   State: " << item.getState() << " | Assigned participant: "
                      << (item.getAssignedParticipant() ? item.getAssignedParticipant()->getName() : "none") << '\n';
            if (passwords.count(id)) {
                std::cout << "   Required key: " << recoveryKey << '\n';
            }

        };
        auto printTasks = [&]() {
            section(instanceId + " | " + instance.getState() + " | TASKS");
            std::cout << "Incident: " << description << '\n';
            for (size_t i = 0; i < blueprint.activities.size(); ++i)
                printTask(blueprint.activities[i].getId(), i + 1);
        };
        offer(navigation.current);
        for (;;) {
            printTasks();
            std::cout << "\n1. Pick task\n2. View participants\n3. Progress, workload, audit and history\n"
                      << "4. Undo last action\n5. Demo guide\n0. Exit\n";
            int choice = choose("Action: ", 0, 5);
            if (!choice) { std::cout << "Session ended. This incident is stored in memory only.\n"; return 0; }
            if (choice == 2) {
                printParticipants(staff, participantKeys);
                choose("0. Back to task list: ", 0, 0);
                continue;
            }
            if (choice == 4) { undo(); continue; }
            if (choice == 5) { guide(); choose("0. Back to task list: ", 0, 0); continue; }
            if (choice == 3) {
                section("PROGRESS, WORKLOAD, AUDIT AND HISTORY");
                ProgressReportVisitor progress;
                WorkloadVisitor workload;
                instance.getRoot()->accept(progress); instance.getRoot()->accept(workload);
                for (const auto& line : progress.getReport()) std::cout << line << '\n';
                for (const auto& line : workload.getSummary()) std::cout << line << '\n';
                printParticipants(staff, participantKeys);
                std::cout << "\nCommands in effect: " << engine.getHistory(instanceId).size() << '\n';
                for (WorkflowAction* command : engine.getHistory(instanceId)) std::cout << "  " << command->getDescription() << '\n';
                std::cout << "Audit (retained after rollback):\n";
                for (const auto& line : engine.getAuditTrail(instanceId)) std::cout << line << '\n';
                std::cout << "Runtime tree (DFS Iterator):\n";
                std::unique_ptr<RuntimeComponentIterator> iterator(instance.getRoot()->createIterator(
                    new DFSTraversalStrategy<RuntimeComponent>()));
                while (iterator->hasNext()) {
                    const auto& component = dynamic_cast<const RuntimeComponent&>(iterator->getNext());
                    std::cout << "  " << component.getName() << " | " << component.getState() << '\n';
                }
                for (const auto& visit : navigation.visits) std::cout << "Visit: " << visit << '\n';
                choose("0. Back to task list: ", 0, 0);
                continue;
            }
            for (;;) {
                printTasks();
                int taskNumber = choose("Pick task (1-5, 0 back): ", 0, static_cast<int>(blueprint.activities.size()));
                if (!taskNumber) break;
                const std::string id = blueprint.activities[taskNumber - 1].getId();
                WorkItem& item = *items.at(id);
                RuntimeComponent& guard = *guards.at(id);
                Participant* actor = item.getAssignedParticipant();
                guard.setActor(actor);
                auto selectCandidate = [&]() -> bool {
                    if (!actor) {
                        actor = assignment.assign(item, staff);
                        if (!actor) {
                            std::cout << "Could not assign '" << item.getName()
                                      << "': no participant has required role " << item.getRequiredRoleId() << ".\n";
                            return false;
                        }
                        std::cout << assignment.getName() << " assignment selected " << actor->getName()
                                  << " (" << actor->getOpenItems() << " unfinished assignments, required role: "
                                  << item.getRequiredRoleId() << ").\n";
                    }
                    guard.setActor(actor);
                    return true;
                };
                for (;;) {
                    section("TASK: " + item.getName());
                    printTask(id, 0);
                    if (actor && actor != item.getAssignedParticipant()) {
                        std::cout << "Candidate participant: " << actor->getName() << " (not assigned)\n";
                        try {
                            guard.checkAccess();
                            std::cout << "Press 1 to finish assigning this participant.\n";
                        } catch (const std::exception& error) {
                            std::cout << "Assignment pending: " << error.what() << ".\n";
                            if (passwords.count(id) && !passwords.at(id)->canModify() &&
                                actor->hasRole(item.getRequiredRoleId()) && dependencyReason(id).empty())
                                std::cout << "Enter 2, then the required key shown above, to finish assignment.\n";
                        }
                    } else if (actor) {
                        std::cout << "Acting participant: " << actor->getName() << " (assigned)\n";
                    } else {
                        std::cout << "No participant assigned. Progress will select one using the assignment strategy.\n";
                    }
                    std::cout << "\n1. Progress (automatic assignment / start / complete / resubmit)\n"
                              << "2. Enter access key\n3. Request corrections\n4. Escalate\n"
                              << "5. Undo last action\n0. Back to task list\n";
                    int action = choose("Task action: ", 0, 5);
                    if (!action) break;
                    if (action == 5) { undo(); break; }
                    if (action == 2) {
                        if (!passwords.count(id)) std::cout << "This task does not require an access key.\n";
                        else if (selectCandidate()) {
                            bool accepted = passwords.at(id)->enterKey(readLine("Portal recovery key: "));
                            if (accepted) participantKeys[actor->getParticipantId()]["portal recovery key"] = recoveryKey;
                            std::cout << (accepted ? "Key accepted for " : "Could not unlock task: incorrect portal recovery key for ")
                                      << actor->getName() << ".\n";
                            if (accepted && actor != item.getAssignedParticipant()) {
                                UndoPoint before = {history.getAll().size(), navigation};
                                const bool assigned = submit(engine, guard, item, std::unique_ptr<WorkflowAction>(
                                    new AssignParticipantCommand(item, actor)), "assign participant");
                                if (assigned) {
                                    undoPoints.push_back(before);
                                    std::cout << actor->getName() << " is now assigned. Press 1 to "
                                              << (item.getState() == "Assigned" ? "start" : "continue") << " the task.\n";
                                } else {
                                    std::cout << "Key accepted, but assignment remains blocked by the requirement above.\n";
                                }
                            }
                        }
                        continue;
                    }
                    UndoPoint before = {history.getAll().size(), navigation};
                    bool ok = false, retryBoundary = false, completed = false;
                    std::string verb;
                    if (action == 1) {
                        if (item.getState() == "Completed") {
                            std::cout << "Could not progress '" << item.getName() << "': task is already Completed.\n";
                            continue;
                        }
                        if (!selectCandidate()) continue;
                        guard.setActor(actor);
                        if (!item.getAssignedParticipant() || actor != item.getAssignedParticipant()) {
                            verb = "assign participant";
                            ok = submit(engine, guard, item, std::unique_ptr<WorkflowAction>(
                                new AssignParticipantCommand(item, actor)), verb);
                        } else if (item.getState() == "Assigned") {
                            verb = "start";
                            ok = submit(engine, guard, item, std::unique_ptr<WorkflowAction>(new StartCommand(&item)), verb);
                        } else if (item.getState() == "Rejected") {
                            verb = "resubmit";
                            ok = submit(engine, guard, item, std::unique_ptr<WorkflowAction>(new makeAvailableCommand(&item)), verb);
                            if (ok) ok = submit(engine, guard, item, std::unique_ptr<WorkflowAction>(
                                new AssignParticipantCommand(item, actor)), "assign corrected work");
                        } else if (item.getState() == "In_Progress") {
                            verb = "complete";
                            // Check decorators before approval or outcome prompts as well as command execution.
                            try { guard.checkAccess(); }
                            catch (const std::exception& error) {
                                std::cout << "Could not complete '" << item.getName() << "': " << error.what() << ".\n";
                                continue;
                            }
                            if (id == "repair") {
                                Request request("repair-approval", "APPROVE", id, instanceId, "administrator",
                                    actor->getParticipantId(), "Authorize portal repair", item.getState(), Data(description));
                                if (validation.authorise(request) != HandlerResult::Handled) {
                                    std::cout << "Could not complete repair: recovery approval was refused.\n";
                                    continue;
                                }
                            }
                            std::string outcome = "done";
                            if (id == "verify") {
                                std::cout << "Student login result: 1. Passed  2. Failed\n";
                                outcome = choose("Observed result: ", 1, 2) == 1 ? "passed" : "failed";
                            }
                            ok = submit(engine, guard, item, std::unique_ptr<WorkflowAction>(new CompleteCommand(&item)), verb);
                            if (ok) {
                                completed = true;
                                navigation.visits.push_back(id + ": " + (id == "verify" ? outcome : "completed"));
                                if (id == "verify") {
                                    navigation.verificationPassed = outcome == "passed";
                                    bus.notify(WorkFlowEvent("VERIFICATION_RESULT", instanceId, id, outcome));
                                }
                                RoutingStrategy* router = id == "verify" ? static_cast<RoutingStrategy*>(&verification) : &sequential;
                                const auto next = router->next(*findNode(*definition, id), *definition, outcome);
                                navigation.current = next.empty() ? "" : next.front()->getId();
                                std::cout << router->getName() << " routes to "
                                          << (navigation.current.empty() ? "end of process" : navigation.current) << ".\n";
                                if (id == "verify" && outcome == "failed") {
                                    rollback(navigation.checkpoints.at("repair"));
                                    require(items.at("repair")->getState() == "Created" &&
                                            items.at("verify")->getState() == "Created", "Retry resets repair and verification");
                                    undoPoints.clear(); retryBoundary = true;
                                    bus.notify(WorkFlowEvent("RETRY", instanceId, "repair", "Repair and verification reset"));
                                    std::cout << "Repair and verification rolled back. A new retry checkpoint is established.\n";
                                }
                                if (!navigation.current.empty()) offer(navigation.current);
                                else {
                                    require(engine.submit(new StageStateCommand(*stageGuard, instance, "COMPLETED")), "Cannot complete case");
                                    bus.notify(WorkFlowEvent("INCIDENT_RESOLVED", instanceId, id, description));
                                    std::cout << "INCIDENT RESOLVED: students can access the learning portal.\n";
                                }
                            }
                        } else {
                            std::cout << "Could not progress '" << item.getName() << "': state " << item.getState()
                                      << " cannot progress through automatic assignment.\n";
                            continue;
                        }
                    } else {
                        guard.setActor(actor);
                        verb = action == 3 ? "request corrections" : "escalate";
                        std::unique_ptr<WorkflowAction> command(action == 3
                            ? static_cast<WorkflowAction*>(new RejectCommand(&item))
                            : static_cast<WorkflowAction*>(new EscalateCommand(&item)));
                        ok = submit(engine, guard, item, std::move(command), verb);
                        if (ok && action == 4) { actor = nullptr; guard.setActor(nullptr); }
                    }
                    if (ok && !retryBoundary) undoPoints.push_back(before);
                    if (!ok) {
                        if (history.getAll().size() > before.historySize) rollback(before.historySize);
                        navigation = before.navigation;
                        std::cout << "Task state, assignee and command history were preserved.\n";
                    }
                    if (completed) break; // Re-list tasks after routing, retry or case completion.
                }
            }
        }
    } catch (const InputClosed&) {
        std::cout << "\nInput closed; demo ended cleanly.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "SCENARIO 1 ERROR: " << error.what() << '\n';
        return 1;
    }
}
