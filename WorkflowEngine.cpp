#include "WorkflowEngine.h"
bool WorkflowEngine:: submit(WorkflowAction* action){
    action->execute();
}
    void WorkflowEngine::undoLast(){
        std::cout<<"WorkActionUndone"<<std::endl;
    }
