#ifndef ASSIGNCOMMAND_H
#define ASSIGNCOMMAND_H
#include "WorkItem.h"
#include "WorkflowAction.h"
#include "Participant.h"
class AssignCommand: public WorkflowAction{
    private:
        WorkItem* receiver;
        Participant* participant;
    public:
        AssignCommand(WorkItem* r, Participant* p);
        bool execute()override;
        void undo()override;
        std::string getDescription()override;
};
#endif