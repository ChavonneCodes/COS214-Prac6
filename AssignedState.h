#ifndef ASSIGNEDSTATE_H
#define ASSIGNEDSTATE_H

#include "WorkItemState.h"
class AssignedState: public WorkItemState{

    public:
       bool start(WorkItem* item) override;
        bool reject(WorkItem* item) override;
       bool cancel(WorkItem* item) override;
        bool escalated(WorkItem* item)override;
       WorkItemState* clone();
       std::string getName()override;
}; 
#endif