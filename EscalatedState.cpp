#include "EscalatedState.h"
#include "Cancelled.h"
#include "WorkItem.h"

void EscalatedState::assign(WorkItem* item, Participant* p){
    item->assign(p);
    item->changeState(new AssignedState());
    
}
       bool EscalatedState:: cancel(WorkItem* item){
        item->changeState(new CancelledState());
        return true;
       }
       WorkItemState*EscalatedState:: clone(){
        return new EscalatedState(*this);
       
       }