/**
 * @file RejectedState.h
 * @brief Defines the RejectedState class.
 */
#ifndef REJECTEDSTATE_H
#define REJECTEDSTATE_H

#include "WorkItemState.h"
/**
 * @class RejectedState
 * @brief ConcreteState for a work item whose assigned owner declined it. It allows
 * makeAvailable (offers the item again; moves to AvailableState) and cancel
 * (moves to CancelledState); every other action is refused. This is a task-
 * level state meaning "declined by the assignee", not an approval outcome,
 * which is carried by routing.
 */
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
       WorkItemState* clone()override;
       /**
     * @return name of RejectedState as a string
     */
       std::string getName()override;
}; 
#endif