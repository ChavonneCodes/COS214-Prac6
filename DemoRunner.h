/**
 * @file DemoRunner.h
 * @brief The code both demo scenarios share: one engine and one run loop.
 *
 * Demo::run() starts a case, walks the routing and produces the reports. It is
 * the same for every process. A Scenario supplies only what is different, which
 * is the point of the demo: TIVIDY is generic, the scenarios are just data.
 */
#ifndef DEMO_RUNNER_H
#define DEMO_RUNNER_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "ApprovalChainDefinition.h"
#include "ApprovalHandler.h"
#include "ApprovalStepDefinition.h"
#include "AuditObserver.h"
#include "CompleteCommand.h"
#include "ConcreteWorkflowBuilder.h"
#include "DemoTrace.h"
#include "EscalateCommand.h"
#include "EventBus.h"
#include "HistoryManager.h"
#include "InMemoryDefinitionRepository.h"
#include "Iterator.h"
#include "NotificationObserver.h"
#include "ProgressReportVisitor.h"
#include "RoleApprovalHandler.h"
#include "RoutingHelper.h"
#include "SubProcessDefinition.h"
#include "StartCommand.h"
#include "TimeoutEscalationHandler.h"
#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "WorkflowEngine.h"
#include "WorkItem.h"
#include "WorkloadVisitor.h"

/** @brief One row of a process definition. An empty stage puts it directly under the root. */
struct Step {
    std::string id;     ///< Unique id of the activity.
    std::string name;   ///< Display name of the activity.
    std::string role;   ///< Id of the role that performs the activity.
    std::string stage;  ///< Stage the activity sits under (empty for directly under the root).
    std::string calls;   ///< Definition this step runs as a sub-process (empty for a normal activity).
};

/** @brief Follows the fixed transitions like SequentialRouting, but can also reach a sub-process node. */
class TreeRouting : public RoutingStrategy {
public:
/**
     * @brief Finds the nodes reached by the fixed transitions leaving a completed node.
     * @param completed The node that has just finished.
     * @param definition The definition whose transitions and tree are searched.
     * 
     * @return Non-owning pointers to the target nodes. 
     */
    std::vector<const WorkflowComponent*> next(const WorkflowComponent& completed, const WorkflowDefinition& definition,
                                               const std::string& /*outcome*/) override {
        std::vector<const WorkflowComponent*> result;
        for (const TransitionDefinition& transition : definition.getTransitions()) {
            if (transition.getFrom() != completed.getId()) continue;
            if (const WorkflowComponent* target = findNode(definition, transition.getTo())) result.push_back(target);
        }
        return result;
    }
    /**
     * @brief Names this strategy.
     * @return The string "Tree".
     */
    std::string getName() const override { return "Tree"; }
};

/** @brief Everything that differs between two processes. */
struct Scenario {
    int number;                  ///< Scenario number shown in the demo output.
    std::string title;           ///< Human-readable scenario title.
    std::string definitionId;    ///< Id of the process definition to run.
    std::string firstStep;       ///< Id of the first activity to run.
    int version;                 ///< Definition version the case is pinned to.
    int budget;   ///< Case data (0 if this process has no budget).
    std::function<void()> define;                                    ///< Stores the definition, if not stored yet.
    std::function<RuntimeComponent*(RuntimeComponent*)> treeWrapper; ///< Extra decorators on the live tree.
    std::function<void(const std::string&)> onStart;                 ///< Runs once the case exists.
    std::map<std::string, std::function<std::string(WorkItem*)>> special; ///< Steps that differ. Returns the outcome.
    std::map<std::string, RoutingStrategy*> routers;                 ///< Steps that do not route sequentially.
    std::vector<Branch> branches;                                    ///< Those routers' arrows, for the GUI.
    std::string calledDefinition;                                    ///< Definition run as a sub-process, if any.
};

/**
 * @brief Makes a stage or activity node for the definition tree.
 * @param type Kind of node (stage, activity or sub-process).
 * @param id   Unique node id.
 * @param name Display name.
 * @param role Id of the responsible role (empty for stages).
 * @return The node, by value.
 */
inline WorkflowNodeData makeNode(NodeType type, const std::string& id, const std::string& name, const std::string& role = "") {
    WorkflowNodeData node;
    node.type = type;
    node.id = id;
    node.name = name;
    node.roleId = role;
    return node;
}

/**
 * @brief Builds a blueprint from a table of steps.
 * @param id       Definition id.
 * @param name     Process name.
 * @param version  Definition version.
 * @param steps    Activities in order. Neighbours with the same stage share a stage node.
 * @param links    Fixed "always" transitions as from, to.
 * @return The finished blueprint, by value.
 */
inline WorkflowBlueprint makeBlueprint(const std::string& id, const std::string& name, int version,
                                const std::vector<Step>& steps,
                                const std::vector<std::pair<std::string, std::string>>& links) {
    WorkflowBlueprint blueprint;
    blueprint.id = id;
    blueprint.name = name;
    blueprint.version = version;
    blueprint.hasRootTree = true;
    blueprint.rootTreeData = makeNode(NodeType::STAGE, "root", name);

    std::set<std::string> roles;
    std::string openStage;
    for (const Step& step : steps) {
        WorkflowNodeData leaf = makeNode(NodeType::ACTIVITY, step.id, step.name, step.role);
        if (step.calls.empty()) {
            if (roles.insert(step.role).second) blueprint.roles.push_back(RoleDefinition(step.role, step.role, 1));
            blueprint.activities.push_back(ActivityDefinition(step.id, step.name, step.role));
        } else {
            leaf = makeNode(NodeType::SUBPROCESS, step.id, step.name);
            leaf.calledDefId = step.calls;
        }
        std::vector<WorkflowNodeData>& top = blueprint.rootTreeData.children;
        if (step.stage.empty()) {
            top.push_back(leaf);
        } else {
            if (step.stage != openStage) {
                std::string stageId = step.stage;
                stageId.erase(std::remove(stageId.begin(), stageId.end(), ' '), stageId.end());
                top.push_back(makeNode(NodeType::STAGE, stageId, step.stage));
            }
            top.back().children.push_back(leaf);
        }
        openStage = step.stage;
    }
    for (const auto& link : links) blueprint.transitions.push_back(TransitionDefinition(link.first, link.second, "always"));
    return blueprint;
}

/**
 * @brief Builds a live approval chain from its definition.
 * @param definition The chain's steps, in order.
 * @param timeoutItem Work item the last link escalates on a timeout (may be null for no timeout link).
 * @param engine     Engine that runs the escalation command.
 * @return The handlers, head first. They must outlive every use of the chain.
 */
inline std::vector<std::unique_ptr<ApprovalHandler>> buildApprovalChain(const ApprovalChainDefinition& definition,
                                                                        WorkItem* timeoutItem, WorkflowEngine* engine) {
    std::vector<std::unique_ptr<ApprovalHandler>> handlers;
    for (const ApprovalStepDefinition& step : definition.getSteps()) {
        handlers.emplace_back(new RoleApprovalHandler(step.getRoleId()));
    }
    if (timeoutItem) handlers.emplace_back(new TimeoutEscalationHandler(timeoutItem, engine));
    for (size_t i = 0; i + 1 < handlers.size(); ++i) handlers[i]->setNext(handlers[i + 1].get());
    return handlers;
}

/** @brief The parts shared by every case: one engine and one run loop. */
class Demo {
public:
    InMemoryDefinitionRepository repository;///< Stores the process definitions. Owns them.
    EventBus bus; ///< Subject that notifies the attached observers.
    HistoryManager history;///< Caretaker holding the executed commands for undo.
    AuditObserver audit;///< Observer that records an audit trail.
    NotificationObserver decisions{"APPROVAL_DECISION"};///< Observer for approval-decision events.
    DemoTrace trace;///< Observer that records moments for the GUI.
    WorkflowEngine engine{&repository, &bus, &history, &audit};///< The engine; uses, but does not own, the parts above.
    TreeRouting sequential;///< Default routing strategy used when a step has no router.
    std::string instanceId;///< Id of the case currently running (empty between cases).
    std::vector<std::string> summary; ///< One line of results per finished case.
    /**
     * @brief Constructs the demo and attaches its observers to the bus.
     *
     * The bus stores raw pointers to audit, decisions and trace. The Demo owns them
     * as members and they are declared before use, so they outlive the bus's use of them.
     * The bus never deletes them.
     */
    Demo() {
        bus.attach(&audit);
        bus.attach(&decisions);
        bus.attach(&trace);   // the GUI is just one more observer
    }

    /**
     * @brief Director + Builder: stores a definition in the repository.
     * @param blueprint The blueprint to build. Taken by value.
     */
    void store(WorkflowBlueprint blueprint) {
        ConcreteWorkflowBuilder builder;
        WorkflowDirector director;
        director.construct(builder, blueprint);
        repository.save(new WorkflowDefinition(builder.build()));
    }

    /**
     * @brief Records a moment for the GUI and prints it. Moments unique to a scenario stand out with >>.
     * @param phase   Phase of the run (for example "define", "run", "report").
     * @param differs True if this moment is unique to the scenario.
     * @param current Id of the current step.
     * @param title   Short title of the moment.
     * @param detail  Description of what happened.
     * @param pattern Design patterns on show (optional).
     * @param call    Engine call that produced it (optional).
     */
    void beat(const std::string& phase, bool differs, const std::string& current, const std::string& title,
              const std::string& detail, const std::string& pattern = "", const std::string& call = "") {
        trace.beat(engine, instanceId, phase, differs, current, title, detail, pattern, call);
        std::cout << (differs ? "  >> " : "     ") << title << ": " << detail
                  << (pattern.empty() || !differs ? "" : "  [" + pattern + "]") << "\n";
    }

    /**
     * @brief Marks a moment that only one scenario has, during a step.
     * @param current Id of the current step.
     * @param title   Short title of the moment.
     * @param detail  Description of what happened.
     * @param pattern Design patterns on show.
     * @param call    Engine call that produced it (optional).
     */
    void unique(const std::string& current, const std::string& title, const std::string& detail,
                const std::string& pattern, const std::string& call = "") {
        beat("run", true, current, title, detail, pattern, call);
    }

    /**
     * @brief The normal path of a step, which is the same in every process.
     * @param item    Non-owning pointer to the work item. The instance keeps ownership.
     * @param outcome Outcome to report (defaults to "approved").
     * @return The outcome, unchanged.
     */
    std::string startAndComplete(WorkItem* item, const std::string& outcome = "approved") {
        engine.submit(new StartCommand(item));
        engine.submit(new CompleteCommand(item));
        beat("run", false, item->getId(), item->getName(), item->getAssignedParticipant()->getName() + " started and completed it",
             "Command + State", "engine.submit(StartCommand), engine.submit(CompleteCommand)");
        return outcome;
    }

    /**
     * @brief A mistaken escalation that is undone. The code is shared, only the step differs.
     * @param item Non-owning pointer to the work item. The instance keeps ownership.
     */
    void escalateAndUndo(WorkItem* item) {
        Participant* owner = item->getAssignedParticipant();
        engine.submit(new EscalateCommand(item));
        std::string during = item->getState();
        engine.undoLast();                 // the command restores its own memento, owner included
        beat("run", false, item->getId(), "Rollback", owner->getName() + " escalated by mistake (" + during +
             "), undo restored " + item->getState(), "Command + Memento + Prototype", "engine.submit(EscalateCommand), engine.undoLast()");
    }

     /**
     * @brief Runs one case from start to reports. This is the same code for every scenario.
     * @param scenario The scenario to run. Not owned. It must stay valid for the whole call.
     */
    void run(Scenario& scenario) {
        std::cout << "\n=== " << scenario.title << " ===\n";
        instanceId.clear();
        if (scenario.define) scenario.define();
        const WorkflowDefinition* definition = repository.load(scenario.definitionId, scenario.version);
        trace.beginCase(scenario.number, scenario.title, *definition, scenario.branches,
                        scenario.calledDefinition.empty() ? nullptr : repository.load(scenario.calledDefinition, 1));
        beat("define", false, "", "Definition", scenario.definitionId + " v" + std::to_string(scenario.version) +
             " stored in the repository", "Builder + Composite", "repository.load(id, version)");

        engine.setTreeWrapper(scenario.treeWrapper);
        instanceId = engine.startWorkflow(scenario.definitionId, scenario.version);
        WorkflowInstance* instance = engine.getInstance(instanceId);
        instance->getData().setValue(std::to_string(scenario.budget));
        bus.notify(WorkFlowEvent("INSTANCE_STARTED", instanceId, "-", "RUNNING"));
        beat("start", false, "", "Case started", instanceId + " pinned to v" + std::to_string(scenario.version) +
             ", least-busy person assigned to each step", "Factory Method + Strategy + Decorator",
             "engine.startWorkflow(id, version)");
        if (scenario.onStart) scenario.onStart(instanceId);

        std::string currentId = scenario.firstStep;
        while (!currentId.empty()) {
            const WorkflowComponent* node = findNode(*definition, currentId);
            std::string outcome = "approved";
            if (auto* call = dynamic_cast<const SubProcessDefinition*>(node)) {
                runSubProcess(*call);
            } else {
                WorkItem* item = instance->findWorkItem(currentId);
                // A Completed item has no way forward, so a routing loop rolls the commands back instead
                while (item->getState() == "Completed" || item->getState() == "In_Progress") engine.undoLast();
                auto step = scenario.special.find(currentId);
                outcome = step != scenario.special.end() ? step->second(item) : startAndComplete(item);
            }

            auto router = scenario.routers.find(currentId);
            RoutingStrategy* routing = router != scenario.routers.end() ? router->second : &sequential;
            std::vector<const WorkflowComponent*> next =
                routing->next(*node, *definition, outcome);
            currentId = next.empty() ? "" : next[0]->getId();
        }

        instance->setState("COMPLETED");
        bus.notify(WorkFlowEvent("INSTANCE_COMPLETED", instanceId, "-", "COMPLETED"));
        report(scenario.title);
        trace.endCase();
    }

private:
    /** @brief Composite: a step that is a whole workflow. The parent waits while the child runs. */
    void runSubProcess(const SubProcessDefinition& call) {
        engine.advance(instanceId);
        beat("run", true, call.getId(), "Sub-process", "The parent pauses while " + call.getCalledDefinitionId() +
             " runs as its own workflow", "Composite", "engine.advance(instanceId)");
        WorkflowInstance* child = engine.getInstance(instanceId + "-" + call.getId());
        for (WorkItem* item : child->getWorkItems()) startAndComplete(item);
        child->setState("COMPLETED");
        engine.advance(instanceId);
        beat("run", true, call.getId(), "Parent resumes", "The child workflow finished, so the parent carries on",
             "Composite", "engine.advance(instanceId)");
    }

    /** @brief The reports are the same for every case: only the data in them differs. */
    void report(const std::string& title) {
        std::unique_ptr<RuntimeComponentIterator> walk(
            engine.getInstance(instanceId)->getRoot()->createIterator(new DFSTraversalStrategy<RuntimeComponent>()));
        std::string order;
        while (walk->hasNext()) {
            order += (order.empty() ? "" : " ") + dynamic_cast<const RuntimeComponent&>(walk->getNext()).getId();
        }
        beat("report", false, "", "Iterator walk", order, "Iterator", "root->createIterator(strategy)");

        ProgressReportVisitor progress;
        WorkloadVisitor workload;
        engine.applyVisitor(instanceId, progress);
        engine.applyVisitor(instanceId, workload);
        beat("report", false, "", "Report", std::to_string(progress.getCompletedItems()) + " of " +
             std::to_string(progress.getTotalItems()) + " steps done, " +
             std::to_string(engine.getHistory(instanceId).size()) + " commands in history, " +
             std::to_string(engine.getAuditTrail(instanceId).size()) + " audit lines",
             "Visitor + Observer", "engine.applyVisitor(id, visitor), engine.getAuditTrail(id)");
        summary.push_back(title + ": " + std::to_string(progress.getCompletedItems()) + " of " +
                          std::to_string(progress.getTotalItems()) + " steps done, " +
                          std::to_string(engine.getHistory(instanceId).size()) + " commands");
    }
};

#endif // DEMO_RUNNER_H
