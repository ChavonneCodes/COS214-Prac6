#include"RejectedState.h"
#include "CancelledState.h"
#include "AvailableState.h"
#include "WorkItem.h"
bool RejectedState::cancel(WorkItem* item) {
    item->changeState(new CancelledState());
    return true;
}
       bool RejectedState::makeAvailable(WorkItem* item){
        item->changeState(new AvailableState());
        return true;
       }
       WorkItemState* RejectedState::clone(){
        return new RejectedState(*this);
       }
        std::string RejectedState::getName(){
   return "Rejected";
 }