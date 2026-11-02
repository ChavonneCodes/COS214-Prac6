/**
 * @file CompleteCommand.h
 * @brief Defines the CompleteCommand class.
 */
#ifndef COMPLETECOMMAND_H
#define COMPLETECOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
#include "WorkItemMemento.h"
#include <string>
#include <vector>
/**
 * @class CompleteCommand
 * @brief ConcreteCommand that finishes an In_Progress work item, moving it to
 * Completed. The owner stays on record but the item stops counting towards
 * their workload. It is refused in any other state. execute() saves a snapshot
 * first and restores it if the action is refused; undo() restores the last
 * snapshot, which also puts the workload back.
 */
class CompleteCommand: public WorkflowAction{
    /**
 * @brief Creates a commanf that completes the given work item.
 * @param r Non-owning pointer to the work item. The caller keeps ownership
 *          and must keep it alive until execute() has run.
 */
    private:
        WorkItem* receiver;
        std::vector<WorkItemMemento> myHistory;
    public:
    /**
 * @brief Creates a command that completes the given work item.
 * @param r Non-owning pointer to the work item. The caller keeps ownership
 *          and must keep it alive until execute() has run.
 */
        CompleteCommand(WorkItem* r);
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