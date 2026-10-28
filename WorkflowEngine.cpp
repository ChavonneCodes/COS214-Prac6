#include "WorkflowEngine.h"
#include "InstanceFactory.h"
#include "LabelledRuntimeDecorator.h"
#include "WorkItem.h"
#include "SubProcessDefinition.h"
#include "WorkflowComponent.h"
#include  "RoutingHelper.h"
#include "ActivityDefinition.h"
#include <functional>
#include "SubProcessInstance.h"


WorkflowEngine::WorkflowEngine(InMemoryDefinitionRepository* repository, EventBus* eventBus,
                               HistoryManager* history, AuditObserver* audit)
    : repository(repository), eventBus(eventBus), history(history), audit(audit){}

WorkflowEngine::~WorkflowEngine(){
    for (auto& entry : instances) delete entry.second;
    for (WorkflowAction* action : submitted) delete action;
}

void WorkflowEngine::setAssignment(AssignmentStrategy* strategy, const std::vector<Participant*>& candidates){
    assignment = strategy;
    this->candidates = candidates;
}

std::string WorkflowEngine::startWorkflow(const std::string& defId, int version){
    if (!repository) return "";
    WorkflowDefinition* definition = repository->load(defId, version);
    if (!definition || !definition->getRoot()) return "";

    // Factory Method + Strategy: build the runtime tree and assign every work item
    std::string instanceId = defId + "-" + std::to_string(++instanceCount);
    RuntimeComponent* tree = InstanceFactory::instantiateTree(instanceId, definition->getRoot(),
                                                              assignment, candidates);
    if (!tree) return "";

    // Decorator: label the tree with the instance and the version it is pinned to
    RuntimeComponent* root = new LabelledRuntimeDecorator(
        tree, "Instance " + instanceId + " (" + defId + " v" + std::to_string(version) + ")");
    WorkflowInstance* instance = new WorkflowInstance(instanceId, defId, version, root, eventBus);

    for (WorkItem* item : instance->getWorkItems()) {
        item->setEventBus(eventBus);                        // from now on, publish every change
    }

    instances[instanceId] = instance;
    instance->setState("RUNNING");
    return instanceId;
}

WorkflowInstance* WorkflowEngine::getInstance(const std::string& instanceId){
    auto found = instances.find(instanceId);
    return found == instances.end() ? nullptr : found->second;
}

bool WorkflowEngine::submit(WorkflowAction* action){
    if (!action) return false;
    submitted.push_back(action);
    bool ok = action->execute();
    if (ok && history) history->push(action);
    return ok;
}

void WorkflowEngine::undoLast(){
    if (!history) return;
    WorkflowAction* last = history->pop();
    if (last) last->undo();
}

std::vector<WorkflowAction*> WorkflowEngine::getHistory(const std::string& instanceId){
    std::vector<WorkflowAction*> result;
    if (!history) return result;
    for (WorkflowAction* action : history->getAll()) {
        if (action->getInstanceId() == instanceId) result.push_back(action);
    }
    return result;
}

std::vector<std::string> WorkflowEngine::getAuditTrail(const std::string& instanceId){
    std::vector<std::string> result;
    if (!audit) return result;
    // AuditObserver writes each line as "... instance=<id> item=..."
    std::string tag = " instance=" + instanceId + " ";
    for (const std::string& line : audit->getLog()) {
        if (line.find(tag) != std::string::npos) result.push_back(line);
    }
    return result;
}

bool WorkflowEngine::applyVisitor(const std::string& instanceId, RuntimeVisitor& visitor){
    WorkflowInstance* instance = getInstance(instanceId);
    if (!instance || !instance->getRoot()) return false;
    instance->getRoot()->accept(visitor);
    return true;
}

WorkItem* WorkflowEngine::step(const std::string& instanceId,
                               const std::string& completedId,
                               const std::string& outcome,
                               RoutingStrategy& router) {
    WorkflowInstance* instance = getInstance(instanceId);
    if (!instance) return nullptr;

    WorkflowDefinition* def = repository->load(instance->getDefinitionId(),
                                               instance->getDefinitionVersion());
    if (!def) return nullptr;

    // --- Step 1: ask the router what comes next ---
    std::vector<const WorkflowComponent*> next;
    if (completedId.empty()) {
        // First step — use the root's first child (or your start logic)
        next.push_back(def->getRoot());
    } else {
        const WorkflowComponent* completed = findNode(*def, completedId);
        if (!completed) return nullptr;
        next = router.next(*completed, *def, outcome);
    }

    if (next.empty()) return nullptr;   // workflow finished

    const WorkflowComponent* node = next[0];

    // --- Step 2: engine inspects the node type ---

    // Case A: sub-process — auto-dispatch
    if (auto* subDef = dynamic_cast<const SubProcessDefinition*>(node)) {
        std::cout << ">>> Engine reached sub-process: " << subDef->getId() << "\n";

        advance(instanceId);   // dispatches the IDLE sub-process

        std::string childId = instanceId + "-" + subDef->getId();
        WorkflowInstance* child = getInstance(childId);
        if (!child) return nullptr;

        // Run the child to completion (simplified — in real engine this blocks)
        // In the demo, the client will process child items via processTask().
        // For automatic running, you would call child->runAll() or similar.

        // Once child is COMPLETED (see below), resume parent:
        if (child->getState() == "COMPLETED") {
            advance(instanceId);   // marks sub-process COMPLETED
            return step(instanceId, subDef->getId(), "approved", router);   // recurse
        }

        // Child not yet done — return the first child WorkItem for the client
        auto childItems = child->getWorkItems();
        for (WorkItem* item : childItems) {
            if (item->getState() != "Completed" && item->getState() != "Cancelled") {
                return item;
            }
        }
        return nullptr;
    }

    // Case B: activity — return the runtime work item
    if (auto* act = dynamic_cast<const ActivityDefinition*>(node)) {
        return instance->findWorkItem(act->getId());
    }

    return nullptr;
}

void WorkflowEngine::advance(const std::string& instanceId) {
    WorkflowInstance* parent = getInstance(instanceId);
    if (!parent || !parent->getRoot()) return;

    // Recursive lambda: walks the entire runtime tree looking for
    // SubProcessInstance nodes in a state that needs advancing.
    std::function<void(RuntimeComponent*)> walk = [&](RuntimeComponent* node) {
        if (!node) return;

        // ── Case 1: this node is a SubProcessInstance ─────────────────────
        if (auto* sub = dynamic_cast<SubProcessInstance*>(node)) {

            // ── Sub-case 1a: IDLE → dispatch the child workflow ──────────
            if (sub->getState() == "IDLE" && sub->getChildInstance() == nullptr) {
                if (!repository) return;

                WorkflowDefinition* childDef = repository->load(
                    sub->getDefinition()->getCalledDefinitionId(),
                    sub->getDefinition()->getCalledVersion());

                if (!childDef) {
                    std::cerr << "[advance] Could not load child definition '"
                              << sub->getDefinition()->getCalledDefinitionId()
                              << "' v" << sub->getDefinition()->getCalledVersion()
                              << "\n";
                    return;
                }

                // Create a unique ID for the child instance
                std::string childId = instanceId + "-" + sub->getId();

                // Instantiate the child runtime tree (Factory + Strategy)
                RuntimeComponent* childTree = InstanceFactory::instantiateTree(
                    childId, childDef->getRoot(), assignment, candidates);

                if (!childTree) {
                    std::cerr << "[advance] Failed to instantiate child tree for '"
                              << sub->getId() << "'\n";
                    return;
                }

                // Create the nested WorkflowInstance
                WorkflowInstance* child = new WorkflowInstance(
                    childId,
                    childDef->getDefinitionId(),
                    childDef->getVersion(),
                    childTree,
                    eventBus);

                child->setState("RUNNING");
                sub->setChildInstance(child);
                sub->setState("RUNNING");

                // Attach the event bus to every child work item so state
                // changes flow into the shared Observer infrastructure.
                for (WorkItem* item : child->getWorkItems()) {
                    item->setEventBus(eventBus);
                }

                // Register in the engine's instance map (engine owns it)
                instances[childId] = child;

                std::cout << "[advance] Dispatched sub-process '"
                          << sub->getId() << "' → child instance " << childId << "\n";
            }

            // ── Sub-case 1b: RUNNING with a COMPLETED child → resume parent ─
            else if (sub->getState() == "RUNNING" && sub->getChildInstance() != nullptr) {
                if (sub->getChildInstance()->getState() == "COMPLETED") {
                    sub->setState("COMPLETED");
                    std::cout << "[advance] Sub-process '" << sub->getId()
                              << "' completed; parent can resume.\n";
                }
            }
        }

        // ── Always recurse into children so nested sub-processes are found ─
        for (RuntimeComponent* childNode : node->getChildren()) {
            walk(childNode);
        }
    };

    walk(parent->getRoot());
}
