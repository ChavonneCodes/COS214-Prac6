#include "EscalatedState.h"
#include "CancelledState.h"
#include "AssignedState.h"
#include "WorkItem.h"

bool EscalatedState::assign(WorkItem* item, Participant* p){
    if (p==nullptr) return false;   
    bind(item, p);                                   
    transition(item, new AssignedState());
    return true;
   
    
    
}
       bool EscalatedState:: cancel(WorkItem* item){
        reduceParticipantCount(item);
        transition(item, new CancelledState());
        return true;
       }
      WorkItemState* EscalatedState::clone(){
        return new EscalatedState(*this);
       }
        std::string EscalatedState::getName(){
   return "Escalated";
 }