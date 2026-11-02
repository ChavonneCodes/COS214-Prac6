/**
 * @file StartCommand.h
 * @brief Defines the StartCommand class.
 */
#ifndef STARTCOMMAND_H
#define STARTCOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
#include "WorkItemMemento.h"
#include <string>
#include <vector>
/**
 * @class StartCommand
 * @brief ConcreteCommand that starts work on an Assigned work item, moving it to
 * In_Progress. It is refused in any other state. execute() saves a snapshot
 * first and restores it if the action is refused; undo() restores the last
 * snapshot.
 */
class StartCommand: public WorkflowAction{
    private:
        WorkItem* receiver;  
        std::vector<WorkItemMemento> myHistory;
    public:
    /**
 * @brief Creates a command that starts the given work item./in progress
 * @param r Non-owning pointer to the work item. The caller keeps ownership
 *          and must keep it alive until execute() has run.
 */
        StartCommand(WorkItem* r);
        /**
         * @brief calls the appropriate method in WorkItem to execute the command
         * @return true if method is executed successfully, false otherwise
         */
        bool execute()override;
        /**
         * @brief restores the memento taken when command was executed
         */
        void undo()override;
        /**
         * @brief obtains description
         * @return description of the command
         */
        std::string getDescription()override;
        /**
         * @brief obtains instance id
         * @return id of the workflow instance the command acts on
         */
        std::string getInstanceId()override;
};
#endif