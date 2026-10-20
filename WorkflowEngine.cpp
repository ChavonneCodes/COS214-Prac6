#include "WorkflowEngine.h"

WorkflowEngine::WorkflowEngine(InMemoryDefinitionRepository* repository, EventBus* eventBus,HistoryManager* history, AuditObserver* audit)
    : repository(repository), eventBus(eventBus), history(history), audit(audit){}

std::string WorkflowEngine::startWorkflow(const std::string& defId, int version){
    // TODO: load definition from repository, create WorkflowInstance, publish "INSTANCE_STARTED"
    (void)defId;
    (void)version;
    return "";
}

WorkflowInstance* WorkflowEngine::getInstance(const std::string& instanceId){
    // TODO: look up instance once WorkflowInstance exists
    (void)instanceId;
    return nullptr;
}

std::vector<WorkflowAction*> WorkflowEngine::getHistory(const std::string& instanceId){
    // TODO: filter history by instanceId
    (void)instanceId;
    return std::vector<WorkflowAction*>();
}

std::vector<std::string> WorkflowEngine::getAuditTrail(const std::string& instanceId){
    // TODO: filter audit log by instanceId
    (void)instanceId;
    return std::vector<std::string>();
}

bool WorkflowEngine:: submit(WorkflowAction* action){
    action->execute();
    return true; //temporary return value, should be based on the action's execution result
}
    void WorkflowEngine::undoLast(){
        std::cout<<"WorkActionUndone"<<std::endl;
    }
