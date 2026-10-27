#ifndef ASSIGNEDSTATE_H
#define ASSIGNEDSTATE_H

#include "WorkItemState.h"
class AssignedState: public WorkItemState{

    public:
    /**
     * @brief An object in AssignedState may move to InProgressState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into InProgressState
     */
         bool start(WorkItem* item) override;
         /**
     * @brief An object in AssignedState may move to RejectedState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into RejectedState
     */
        bool reject(WorkItem* item) override;
        /**
     * @brief An object in AssignedState may move to CancelledState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into CancelledState
     */
       bool cancel(WorkItem* item) override;
       /**
     * @brief An object in AssignedState may move to EscalatedState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into EscalatedState
     */
        bool escalated(WorkItem* item)override;
        /**
     * @return copy of the AssignedState object
     */
       WorkItemState* clone();
       /**
     
     * @return returns name of state as a string
     */
       std::string getName()override;
}; 
#endif