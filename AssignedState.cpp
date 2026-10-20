#include "AssignedState.h"
#include "InProgressState.h"
#include "RejectedState.h"
#include "CancelledState.h"
#include "EscalatedState.h"
#include "WorkItem.h"



bool AssignedState::start(WorkItem* item){
    item->changeState(new InProgressState());
    return true;
    
}
bool AssignedState::reject(WorkItem* item){
   item->changeState(new RejectedState());
   return true;
}
bool AssignedState::cancel(WorkItem* item) {

   item->changeState(new CancelledState());
   return true;
}
bool AssignedState::escalated(WorkItem* item){
   
    item->changeState(new EscalatedState());
    return true;
}
 WorkItemState*AssignedState:: clone(){
    return new AssignedState(*this);
 }
 std::string AssignedState::getName(){
   return "Assigned";
 }