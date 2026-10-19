#ifndef CREATEDSTATE_H
#define CREATEDSTATE_H

#include "WorkItemState.h"
class CreatedState: public WorkItemState{

    public:
       bool cancel(WorkItem* item) override;
       bool makeAvailable(WorkItem* item)override;
       WorkItemState* clone();
}; 
#endif