#ifndef FAILEDSTATE_H
#define FAILEDSTATE_H

#include "WorkItemState.h"
class FailedState: public WorkItemState{

    public:
       bool cancel(WorkItem* item) override;
       bool makeAvailable(WorkItem* item)override;
       WorkItemState* clone();
}; 
#endif