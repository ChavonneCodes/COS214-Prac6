#include "WorkflowEngine.h"
#include "InstanceFactory.h"
#include "LabelledRuntimeDecorator.h"
#include "WorkItem.h"

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
