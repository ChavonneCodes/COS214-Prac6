/**
 * @file EscalatedState.h
 * @brief Defines the EscalatedState class.
 */
#ifndef ESCALATEDSTATE_H
#define ESCALATEDSTATE_H

#include "WorkItemState.h"
/**
 * @class EscalatedState
 * @brief ConcreteState for a work item that has been handed over for someone else to
 * pick up, for example after a timeout. The previous owner has been released.
 * It allows assign (a new owner takes it; moves to AssignedState) and cancel
 * (moves to CancelledState); every other action is refused.
 */
class EscalatedState: public WorkItemState{

    public:
    /**
     * @brief An object in EscalatedState may move to AssignedState
     * @param item work item which undergoes state change passed as a reference
     * @param p the participant the work item is assigned to
     * @return true, the work item has successfully transitioned into AssignedState
     */
        bool assign(WorkItem* item, Participant* p)override;
        /**
     * @brief An object in EscalatedState may move to CancelledState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into CancelledState
     */
       bool cancel(WorkItem* item) override;
       /**
     * @return copy of the EscalatedState object
     */
       WorkItemState* clone()override;
       /**

     * @return name of EscalatedState as a string
     */
       std::string getName()override;
}; 
#endif