#ifndef FAILCOMMAND_H
#define FAILCOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
#include "WorkItemMemento.h"
#include <vector>
#include <string>
class FailCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
        std::vector<WorkItemMemento> myHistory;
    public:
        FailCommand(WorkItem* r);
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