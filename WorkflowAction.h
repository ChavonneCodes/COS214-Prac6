/**
 * @file WorkflowAction.h
 * @brief implements command interface
 */
#ifndef WORKFLOWACTION_H
#define WORKFLOWACTION_H
#include <string>
#include <iostream>
/**
 * @class WorkflowAction
 * @brief This is the Command participant. Every action performed on a work item is an
 * object that can run itself (execute), reverse itself (undo) and describe
 * itself (getDescription, getInstanceId). WorkflowEngine::submit is the
 * invoker: it runs the command and, only if it succeeded, hands it to the
 * HistoryManager so it can be undone. The receiver is the WorkItem, which
 * passes the request on to its current state.
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