#ifndef CANCELCOMMAND_H
#define CANCELCOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
class CancelCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
    public:
        CancelCommand(WorkItem* r);
        bool execute()override;
        void undo()override;
        std::string getDescription()override;
        std::string getInstanceId()override;
};
#endif