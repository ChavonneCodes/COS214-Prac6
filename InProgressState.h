/**
 * @file InProgressState.h
 * @brief Defines the InProgressState class.
 */
#ifndef INPROGRESSSTATE_H
#define INPROGRESSSTATE_H

#include "WorkItemState.h"
/**
 * @class InProgressState
 * @brief ConcreteState for a work item that its owner is currently working on. It
 * allows complete (moves to CompletedState, ending the owner's workload but
 * keeping them on record), fail (moves to FailedState and the owner is kept so
 * they can retry), escalate (the owner is released; moves to EscalatedState)
 * and cancel (moves to CancelledState).
 */
class InProgressState: public WorkItemState{

    public:
    /**
     * @brief An object in InProgressState may move to CompletedState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into CompletedState
     */
       bool complete(WorkItem* item)override;
       /**
     * @brief An object in InProgressState may move to CancelledState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into CancelledState
     */
       bool cancel(WorkItem* item) override;
       /**
     * @brief An object in InProgressState may move to EscalatedState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into EscalatedState
     */
        bool escalated(WorkItem* item)override;
        /**
     * @brief An object in InProgressState may move to FailedState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into FailedState
     */
        bool fail(WorkItem* item)override;
       /**
     * @return copy of the InProgressState object
     */
       WorkItemState* clone()override;
       /**
     
     * @return name of InProgressState as a string
     */
       std::string getName()override;
}; 
#endif