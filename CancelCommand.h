/**
 * @file CancelCommand.h
 * @brief Defines the CancelCommand class.
 */
#ifndef CANCELCOMMAND_H
#define CANCELCOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
#include "WorkItemMemento.h"
#include <string>
#include <vector>
/**
 * @class CancelCommand
 * @brief ConcreteCommand that abandons a work item, moving it from any non-final
 * state to Cancelled. The owner, if any, stays on record but the item stops
 * counting towards their workload. Completed and Cancelled items refuse it.
 * execute() saves a snapshot first and restores it if the action is refused;
 * undo() restores the last snapshot, which also puts the workload back.
 */
class CancelCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
        std::vector<WorkItemMemento> myHistory;
    public:
        /**
 * @brief Creates a command that cancels the given work item.
 * @param r Non-owning pointer to the work item. The caller keeps ownership
 *          and must keep it alive until execute() has run.
 */
        CancelCommand(WorkItem* r);
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