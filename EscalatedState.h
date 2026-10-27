#ifndef ESCALATEDSTATE_H
#define ESCALATEDSTATE_H

#include "WorkItemState.h"
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
     
     * @return copy of EscalatedState object, used to store state in memento
     */
       WorkItemState* clone();
       /**

     * @return name of EscalatedState as a string
     */
       std::string getName()override;
}; 
#endif