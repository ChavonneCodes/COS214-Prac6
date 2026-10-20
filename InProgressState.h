#ifndef INPROGRESSSTATE_H
#define INPROGRESSSTATE_H

#include "WorkItemState.h"
class InProgressState: public WorkItemState{

    public:
       bool complete(WorkItem* item)override;
       bool cancel(WorkItem* item) override;
        bool escalated(WorkItem* item)override;
        bool fail(WorkItem* item)override;
       WorkItemState* clone();
       std::string getName()override;
}; 
#endif