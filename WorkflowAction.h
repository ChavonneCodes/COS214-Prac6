#ifndef WORKFLOWACTION_H
#define WORKFLOWACTION_H
#include <string>
#include <iostream>
/**
         * @brief abstract interface for commands. Every command which inherits from WorkflowAction overrides these methods
         */
class WorkflowAction{
public:
/**
         * @brief calls the appropriate method in WorkItem to execute the command
         * @return true if method is executed successfully, false otherwise
         */
    virtual bool execute()=0;
    /**
         * @brief restores the memento taken when command was executed
         */
    virtual void undo() =0;
    /**
         * @brief obtains description
         * @return description of the command
         */
    virtual std::string getDescription()=0;
    /**
         * @brief obtains instance id
         * @return id of the workflow instance the command acts on
         */
    virtual std::string getInstanceId()=0; // workflow instance the command acts on
    /**
         * @brief virtual destructor
         */
    virtual ~WorkflowAction(){};

};
#endif