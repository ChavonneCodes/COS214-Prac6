#ifndef CREATEDSTATE_H
#define CREATEDSTATE_H

#include "WorkItemState.h"
class CreatedState: public WorkItemState{

    public:
        /**
     * @brief An object in CreatedState may move to CancelledState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into CancelledState
     */
       bool cancel(WorkItem* item) override;
       /**
     * @brief An object in CreatedState may move to AvailableState
     * @param item work item which undergoes state change passed as a reference
     * @return true, the work item has successfully transitioned into AvailableState
     */
       bool makeAvailable(WorkItem* item)override;
       /**
     * @return copy of CreatedState object, used to store state in memento
     */
       WorkItemState* clone();
       /**
     * @return name of CreatedState as a string
     */
       std::string getName()override;
}; 
#endif