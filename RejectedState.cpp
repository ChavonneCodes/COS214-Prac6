#include"RejectedState.h"
#include "CancelledState.h"
#include "AvailableState.h"
#include "WorkItem.h"
bool RejectedState::cancel(WorkItem* item) {
    release(item);
    transition(item, new CancelledState());
    return true;
}
       bool RejectedState::makeAvailable(WorkItem* item){
        transition(item, new AvailableState());
        return true;
       }
       WorkItemState* RejectedState::clone(){
        return new RejectedState(*this);
       }
        std::string RejectedState::getName(){
   return "Rejected";
 }