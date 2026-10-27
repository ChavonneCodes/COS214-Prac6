#ifndef CANCELCOMMAND_H
#define CANCELCOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
class CancelCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
    public:
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