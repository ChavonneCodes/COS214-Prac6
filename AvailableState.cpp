#include "AvailableState.h"
#include "AssignedState.h"
#include "CancelledState.h"
#include "EscalatedState.h"
#include "WorkItem.h"


 bool AvailableState::assign(WorkItem* item, Participant* p){
  if (p == nullptr) return false;        
    item->setParticipant(p);                  
    item->changeState(new AssignedState());   
    return true;
    
   
 }
 bool AvailableState::cancel(WorkItem* item){
   item->changeState(new CancelledState());
   return true;
   
}
 bool AvailableState::escalated(WorkItem* item){
   item->changeState(new EscalatedState());
   return true;
  
 }
 WorkItemState* AvailableState::clone(){
    return new AvailableState(*this);
    

 }
  std::string AvailableState::getName(){
   return "Available";
 }