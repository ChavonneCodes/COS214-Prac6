/**
 * @file RejectCommand.h
 * @brief Defines the RejectCommand class.
 */
#ifndef REJECTCOMMAND_H
#define REJECTCOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
#include "WorkItemMemento.h"
#include <vector>
#include <string>
/**
 * @class RejectCommand
 * @brief ConcreteCommand for an assignee declining an Assigned work item, moving it
 * to Rejected. The owner is kept until the item is offered again. It is
 * refused in any other state. execute() saves a snapshot first and restores it
 * if the action is refused; undo() restores the last snapshot.
 */

class RejectCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
        std::vector<WorkItemMemento> myHistory;
    public:
    /**
 * @brief Creates a command that rejects the given work item.
 * @param r Non-owning pointer to the work item. The caller keeps ownership
 *          and must keep it alive until execute() has run.
 */
        RejectCommand(WorkItem* r);
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