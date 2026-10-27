#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include "WorkflowAction.h"
#include "WorkItemMemento.h"
#include <iostream>
#include <vector>

/*
class HistoryManager{
    public:
        void push(WorkflowAction*);
        WorkflowAction* pop();
        std::vector<WorkflowAction*> getAll();

    private:
        std::vector<WorkflowAction*> executed;  
};
*/

class HistoryManager{
    public:
        void save(WorkItemMemento&);
        WorkItemMemento pop();
        std::vector<WorkItemMemento> getAll();
        WorkItemMemento getLast();

    private:
        std::vector<WorkItemMemento> mementos;  
};
#endif