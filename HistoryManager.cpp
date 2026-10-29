#include "HistoryManager.h"
using namespace std;

vector<WorkflowAction*> HistoryManager::getAll(){
    return executed;
}

void HistoryManager::push(WorkflowAction* action){
    executed.push_back(action);
}

WorkflowAction* HistoryManager::pop(){
    WorkflowAction* a1;
    if(!executed.empty()){
        a1 = executed.at(executed.size() - 1);
        executed.pop_back();
        return a1;
    } 

    a1 = nullptr;
    return a1;
}

