/**
 * @file AssignCommand.h
 * @brief Defines the AssignCommand class.
 */
#ifndef ASSIGNCOMMAND_H
#define ASSIGNCOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
#include "Participant.h"
#include "WorkItemMemento.h"
#include <string>
#include <vector>
/**
 * @class AssignCommand
 * @brief Defines the AssignCommand class.
 *
 * ConcreteCommand that gives a work item to a participant. It moves an
 * Available or Escalated item to Assigned, binding the participant as owner
 * and counting the item in their workload. It is refused in any other state,
 * or when the participant is missing. execute() saves a snapshot first and
 * restores it if the action is refused; undo() restores the last snapshot
 * (state, owner and workload).
 */
class AssignCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
        Participant* participant;
        std::vector<WorkItemMemento> myHistory;
    public:
    /**
 * @brief Creates a command that assigns the given work item.
 * @param r Non-owning pointer to the work item. The caller keeps ownership
 *          and must keep it alive until execute() has run.
 *@param p participant it must be assigned to
 */
        AssignCommand(WorkItem* r, Participant* p);
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