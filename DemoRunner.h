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
#include "SequentialRouting.h"
#include "StartCommand.h"
#include "TimeoutEscalationHandler.h"
#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "WorkflowEngine.h"
#include "WorkItem.h"
#include "WorkloadVisitor.h"

/** @brief One row of a process definition. An empty stage puts the activity directly under the root. */
struct Step {
    std::string id, name, role, stage;
};

/** @brief Everything that differs between two processes. */
struct Scenario {
    int number;
    std::string title, definitionId, firstStep;
    int version;
    int budget;   ///< Case data (0 if this process has no budget).
    std::function<void()> define;                                    ///< Stores the definition, if not stored yet.
    std::function<RuntimeComponent*(RuntimeComponent*)> treeWrapper; ///< Extra decorators on the live tree.
    std::function<void(const std::string&)> onStart;                 ///< Runs once the case exists.
    std::map<std::string, std::function<std::string(WorkItem*)>> special; ///< Steps that differ. Returns the outcome.
    std::map<std::string, RoutingStrategy*> routers;                 ///< Steps that do not route sequentially.
    std::vector<Branch> branches;                                    ///< Those routers' arrows, for the GUI.
};

/** @brief Makes a stage or activity node for the definition tree. */
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
        if (roles.insert(step.role).second) blueprint.roles.push_back(RoleDefinition(step.role, step.role, 1));
        blueprint.activities.push_back(ActivityDefinition(step.id, step.name, step.role));
        WorkflowNodeData leaf = makeNode(NodeType::ACTIVITY, step.id, step.name, step.role);
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
    InMemoryDefinitionRepository repository;
    EventBus bus;
    HistoryManager history;
    AuditObserver audit;
    NotificationObserver decisions{"APPROVAL_DECISION"};
    DemoTrace trace;
    WorkflowEngine engine{&repository, &bus, &history, &audit};
    SequentialRouting sequential;
    std::string instanceId;
    std::vector<std::string> summary; ///< One line of results per finished case.

    Demo() {
        bus.attach(&audit);
        bus.attach(&decisions);
        bus.attach(&trace);   // the GUI is just one more observer
    }

    /** @brief Director + Builder: stores a definition in the repository. */
    void store(WorkflowBlueprint blueprint) {
        ConcreteWorkflowBuilder builder;
        WorkflowDirector director;
        director.construct(builder, blueprint);
        repository.save(new WorkflowDefinition(builder.build()));
    }

    /** @brief Records a moment for the GUI and prints it. Moments unique to a scenario stand out with >>. */
    void beat(const std::string& phase, bool differs, const std::string& current, const std::string& title,
              const std::string& detail, const std::string& pattern = "", const std::string& call = "") {
        trace.beat(engine, instanceId, phase, differs, current, title, detail, pattern, call);
        std::cout << (differs ? "  >> " : "     ") << title << ": " << detail
                  << (pattern.empty() || !differs ? "" : "  [" + pattern + "]") << "\n";
    }

    /** @brief Marks a moment that only one scenario has, during a step. */
    void unique(const std::string& current, const std::string& title, const std::string& detail,
                const std::string& pattern, const std::string& call = "") {
        beat("run", true, current, title, detail, pattern, call);
    }

    /** @brief The normal path of a step, which is the same in every process. */
    std::string startAndComplete(WorkItem* item, const std::string& outcome = "approved") {
        engine.submit(new StartCommand(item));
        engine.submit(new CompleteCommand(item));
        beat("run", false, item->getId(), item->getName(), item->getAssignedParticipant()->getName() + " started and completed it",
             "Command + State", "engine.submit(StartCommand), engine.submit(CompleteCommand)");
        return outcome;
    }

    /** @brief A mistaken escalation that is undone. The code is shared, only the step differs. */
    void escalateAndUndo(WorkItem* item) {
        Participant* owner = item->getAssignedParticipant();
        engine.submit(new EscalateCommand(item));
        std::string during = item->getState();
        engine.undoLast();                 // the command restores its own memento, owner included
        beat("run", false, item->getId(), "Rollback", owner->getName() + " escalated by mistake (" + during +
             "), undo restored " + item->getState(), "Command + Memento + Prototype", "engine.submit(EscalateCommand), engine.undoLast()");
    }

    /** @brief Runs one case from start to reports. This is the same code for every scenario. */
    void run(Scenario& scenario) {
        std::cout << "\n=== " << scenario.title << " ===\n";
        instanceId.clear();
        if (scenario.define) scenario.define();
        const WorkflowDefinition* definition = repository.load(scenario.definitionId, scenario.version);
        trace.beginCase(scenario.number, scenario.title, *definition, scenario.branches);
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
            WorkItem* item = instance->findWorkItem(currentId);
            // A Completed item has no way forward, so a routing loop rolls the commands back instead
            while (item->getState() == "Completed" || item->getState() == "In_Progress") engine.undoLast();
            auto step = scenario.special.find(currentId);
            std::string outcome = step != scenario.special.end() ? step->second(item) : startAndComplete(item);

            auto router = scenario.routers.find(currentId);
            RoutingStrategy* routing = router != scenario.routers.end() ? router->second : &sequential;
            std::vector<const WorkflowComponent*> next =
                routing->next(*findNode(*definition, currentId), *definition, outcome);
            currentId = next.empty() ? "" : next[0]->getId();
        }

        instance->setState("COMPLETED");
        bus.notify(WorkFlowEvent("INSTANCE_COMPLETED", instanceId, "-", "COMPLETED"));
        report(scenario.title);
        trace.endCase();
    }

private:
    /** @brief The reports are the same for every case: only the data in them differs. */
    void report(const std::string& title) {
        std::unique_ptr<RuntimeComponentIterator> walk(
            engine.getInstance(instanceId)->getRoot()->createIterator(new DFSTraversalStrategy<RuntimeComponent>()));
        std::string order;
        while (walk->hasNext()) {
            order += (order.empty() ? "" : " ") + dynamic_cast<const RuntimeComponent&>(walk->getNext()).getId();
        }
        beat("report", false, "", "Walk", order, "Iterator", "root->createIterator(strategy)");

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
