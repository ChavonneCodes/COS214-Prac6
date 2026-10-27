#ifndef REJECTEDSTATE_H
#define REJECTEDSTATE_H

#include "WorkItemState.h"
class RejectedState: public WorkItemState{

    public:
    /**
     * @brief An object in RejectedState may move to CancelledState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into CancelledState
     */
       bool cancel(WorkItem* item) override;
       /**
     * @brief An object in RejectedState may move to AvailableState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into AvailableState
     */
       bool makeAvailable(WorkItem* item)override;
       /**
     
     * @return copy of RejectedState , used to store state in memento
     */
       WorkItemState* clone();
       /**
     * @return name of RejectedState as a string
     */
       std::string getName()override;
}; 
#endif