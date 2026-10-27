#ifndef AVAILABLESTATE_H
#define AVAILABLESTATE_H

#include "WorkItemState.h"
class AvailableState: public WorkItemState{

    public:
    /**
     * @brief An object in AvailableState may move to AssignedState
     * @param item work item which undergoes state change passed as a reference
     * @param p participant who the work item is assigned to
     * @return true, the work item has successfully transitioned into AssignedState, false if any pointer is null
     */
       bool assign(WorkItem* item, Participant* p)override;
    /**
     * @brief An object in AvailableState may move to CancelledState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into CancelledState
     */

       bool cancel(WorkItem* item) override;
       /**
     * @brief An object in AvailableState may move to EscalatedState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into EscalatedState
     */
        bool escalated(WorkItem* item)override;
        /**
     * @return copy of AvailableState object, used to store state in memento (WorkItemMemento)
     */
       WorkItemState* clone();
       /**
     * @return name of AvailableState as a string
     */
       std::string getName()override;
}; 
#endif