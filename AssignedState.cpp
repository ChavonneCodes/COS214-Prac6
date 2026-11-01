#include "AssignedState.h"
#include "InProgressState.h"
#include "RejectedState.h"
#include "CancelledState.h"
#include "EscalatedState.h"
#include "WorkItem.h"



bool AssignedState::start(WorkItem* item){
    transition(item, new InProgressState());
    
    return true;
    
    
}
bool AssignedState::reject(WorkItem* item){
   transition(item, new RejectedState());
   return true;
}
bool AssignedState::cancel(WorkItem* item) {
   reduceParticipantCount(item);
   transition(item, new CancelledState());
   return true;
}
bool AssignedState::escalated(WorkItem* item){
    release(item);
    transition(item, new EscalatedState());
    return true;
}
WorkItemState* AssignedState::clone(){
        return new AssignedState(*this);
       }
 
 std::string AssignedState::getName(){
   return "Assigned";
 }