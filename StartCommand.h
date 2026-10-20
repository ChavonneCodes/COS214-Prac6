#ifndef STARTCOMMAND_H
#define STARTCOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"

class StartCommand: public WorkflowAction{
    private:
        WorkItem* receiver;  
    public:
        StartCommand(WorkItem* r);
        bool execute()override;
        void undo()override;
        std::string getDescription()override;
};
#endif