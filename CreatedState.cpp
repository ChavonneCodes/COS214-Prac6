#include "CreatedState.h"
#include "AvailableState.h"
#include "CancelledState.h"
#include "WorkItem.h"

bool CreatedState::cancel(WorkItem* item) {
    item->changeState( new CancelledState());
    return true;
   
}
       bool CreatedState:: makeAvailable(WorkItem* item){
        item->changeState( new AvailableState());
       return true;
       }
       WorkItemState* CreatedState::clone(){
        return new CreatedState(*this);
       }
        std::string CreatedState::getName(){
   return "Created";
 }