#include "AvailableState.h"
#include "AssignedState.h"
#include "CancelledState.h"
#include "EscalatedState.h"
#include "WorkItem.h"

 void AvailableState::assign(WorkItem* item, Participant* p){
   item->assign(p);
   item->changeState(new AssignedState());
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
    return true;

 }