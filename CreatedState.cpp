#include "CreatedState.h"
#include "AvailableState.h"
#include "CancelledState.h"
#include "WorkItem.h"

bool CreatedState::cancel(WorkItem* item) {
    release(item);
    transition(item, new CancelledState());
    return true;
   
}
       bool CreatedState:: makeAvailable(WorkItem* item){
        transition(item, new AvailableState());
       return true;
       }
       WorkItemState* CreatedState::clone(){
        return new CreatedState(*this);
       }
       
        std::string CreatedState::getName(){
   return "Created";
 }