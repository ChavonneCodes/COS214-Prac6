#include "InProgressState.h"
#include "CompletedState.h"
#include "CancelledState.h"
#include "EscalatedState.h"
#include "FailedState.h"
#include "WorkItem.h"

bool InProgressState::complete(WorkItem* item){
    item->changeState(new CompletedState());
    return true;
}
bool InProgressState::cancel(WorkItem* item){
    item->changeState(new CancelledState());
    return true;
}
bool InProgressState::escalated(WorkItem* item){
    item->changeState(new EscalatedState());
    return true;
}
bool InProgressState::fail(WorkItem* item){
    item->changeState(new FailedState());
    return true;
}
WorkItemState* InProgressState::clone(){
    return new InProgressState(*this);
}
 std::string InProgressState::getName(){
   return "In_Progress";
 }