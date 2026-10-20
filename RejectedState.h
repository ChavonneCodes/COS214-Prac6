#ifndef REJECTEDSTATE_H
#define REJECTEDSTATE_H

#include "WorkItemState.h"
class RejectedState: public WorkItemState{

    public:
       bool cancel(WorkItem* item) override;
       bool makeAvailable(WorkItem* item)override;
       WorkItemState* clone();
       std::string getName()override;
}; 
#endif