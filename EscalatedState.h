#ifndef ESCALATEDSTATE_H
#define ESCALATEDSTATE_H

#include "WorkItemState.h"
class EscalatedState: public WorkItemState{

    public:
        void assign(WorkItem* item, Participant* p)override;
       bool cancel(WorkItem* item) override;
       WorkItemState* clone();
}; 
#endif