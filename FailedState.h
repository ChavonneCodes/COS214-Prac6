/**
 * @file FailedState.h
 * @brief Defines the FailedState class.
 */
#ifndef FAILEDSTATE_H
#define FAILEDSTATE_H

#include "WorkItemState.h"
/**
 * @class FailedState
 * @brief ConcreteState for a work item whose work went wrong while it was in
 * progress. It allows makeAvailable (a retry; moves back to AvailableState)
 * and cancel (moves to CancelledState); every other action is refused. The
 * owner is kept, so a retry can go back to the same person.
 */
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
     * @return copy of the FailedState object
     */
       WorkItemState* clone()override;
       /**
     
     * @return name of FailedState as a string
     */
       std::string getName()override;
}; 
#endif