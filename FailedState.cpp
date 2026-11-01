#include "FailedState.h"
#include "CancelledState.h"
#include "AvailableState.h"
#include "WorkItem.h"
bool FailedState:: cancel(WorkItem* item){
    reduceParticipantCount(item);
    transition(item, new CancelledState());
    return true;
}
       bool FailedState:: makeAvailable(WorkItem* item){
        transition(item, new AvailableState());
        return true;
       }
       WorkItemState* FailedState::clone(){
        return new FailedState(*this);
       }
      
        std::string FailedState::getName(){
   return "Failed";
 }