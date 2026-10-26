#ifndef REJECTCOMMAND_H
#define REJECTCOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
class RejectCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
    public:
        RejectCommand(WorkItem* r);
        bool execute()override;
        void undo()override;
        std::string getDescription()override;
        std::string getInstanceId()override;
};
#endif