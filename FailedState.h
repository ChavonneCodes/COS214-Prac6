#ifndef FAILEDSTATE_H
#define FAILEDSTATE_H

#include "WorkItemState.h"
class FailedState: public WorkItemState{

    public:
    /**
     * @brief An object in FailedState may move to CancelledState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into CancelledState
     */
       bool cancel(WorkItem* item) override;
       /**
     * @brief An object in FailedState may move to AvailableState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into AvailableState
     */
       bool makeAvailable(WorkItem* item)override;
       /**
     * @return tcopy of FailedState, used to store state in memento
     */
       WorkItemState* clone();
       /**
     
     * @return name of FailedState as a string
     */
       std::string getName()override;
}; 
#endif