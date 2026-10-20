#include "WorkflowEngine.h"
bool WorkflowEngine:: submit(WorkflowAction* action){
    action->execute();
    return true; //temporary return value, should be based on the action's execution result
}
    void WorkflowEngine::undoLast(){
        std::cout<<"WorkActionUndone"<<std::endl;
    }
