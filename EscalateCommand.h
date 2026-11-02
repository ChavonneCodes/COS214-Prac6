/**
 * @file EscalateCommand.h
 * @brief Defines the EscalateCommand class.
 */
#ifndef ESCALATECOMMAND_H
#define ESCALATECOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
#include "Participant.h"
#include "WorkItemMemento.h"
#include <string>
#include <vector>
/**
 * @class EscalateCommand
 * @brief ConcreteCommand that hands a work item over for someone else to pick up,
 * moving an Available, Assigned or In_Progress item to Escalated and releasing
 * its owner. It is refused in any other state. The timeout handler submits it
 * through the engine, so escalations appear in the history. execute() saves a
 * snapshot first and restores it if the action is refused; undo() gives the
 * item back to its previous owner.
 */
class EscalateCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
        std::vector<WorkItemMemento> myHistory;
    public:
    /**
 * @brief Creates a command that escalates the given work item.
 * @param r Non-owning pointer to the work item. The caller keeps ownership
 *          and must keep it alive until execute() has run.
 */
        EscalateCommand(WorkItem* r);
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