#ifndef WORKFLOWENGINE_H
#define WORKFLOWENGINE_H
#include "WorkflowAction.h"

class WorkflowEngine{
public:
    bool submit(WorkflowAction* action);
    void undoLast();

};
#endif