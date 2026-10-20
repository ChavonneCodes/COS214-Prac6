#ifndef AVAILABLESTATE_H
#define AVAILABLESTATE_H

#include "WorkItemState.h"
class AvailableState: public WorkItemState{

    public:
        bool assign(WorkItem* item, Participant* p)override;
       bool cancel(WorkItem* item) override;
        bool escalated(WorkItem* item)override;
       WorkItemState* clone();
       std::string getName()override;
}; 
#endif