#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include "WorkflowAction.h"
#include <iostream>
#include <vector>

class HistoryManager{
    public:
        void push(WorkflowAction*);
        WorkflowAction* pop();
        std::vector<WorkflowAction*> getAll();

    private:
        std::vector<WorkflowAction*> executed;  
};

#endif