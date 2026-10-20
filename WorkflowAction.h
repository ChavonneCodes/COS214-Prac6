#ifndef WORKFLOWACTION_H
#define WORKFLOWACTION_H
#include <string>
#include <iostream>

class WorkflowAction{
public:
    virtual bool execute()=0;
    virtual void undo() =0;
    virtual std::string getDescription()=0;
    virtual ~WorkflowAction(){};

};
#endif