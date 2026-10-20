#ifndef ESCALATECOMMAND_H
#define ESCALATECOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
class EscalateCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
        std::string targetRoleId;
    public:
        EscalateCommand(WorkItem* r, std::string& id);
        bool execute()override;
        void undo()override;
        std::string getDescription()override;
};
#endif