#ifndef COMPLETECOMMAND_H
#define COMPLETECOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
class CompleteCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
    public:
        CompleteCommand(WorkItem* r);
        bool execute()override;
        void undo()override;
        std::string getDescription()override;
};
#endif