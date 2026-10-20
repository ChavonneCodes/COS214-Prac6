#ifndef ESCALATEDSTATE_H
#define ESCALATEDSTATE_H

#include "WorkItemState.h"
class EscalatedState: public WorkItemState{

    public:
        bool assign(WorkItem* item, Participant* p)override;
       bool cancel(WorkItem* item) override;
       WorkItemState* clone();
       std::string getName()override;
}; 
#endif