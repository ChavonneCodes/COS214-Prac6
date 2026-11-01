#include "InProgressState.h"
#include "CompletedState.h"
#include "CancelledState.h"
#include "EscalatedState.h"
#include "FailedState.h"
#include "WorkItem.h"

bool InProgressState::complete(WorkItem* item){
    reduceParticipantCount(item); // lowers open-item for workload-based assignment
    transition(item, new CompletedState());
    return true;
}
bool InProgressState::cancel(WorkItem* item){
    reduceParticipantCount(item);
    transition(item, new CancelledState());
    return true;
}
bool InProgressState::escalated(WorkItem* item){
    release(item);
    transition(item, new EscalatedState());
    return true;
}
bool InProgressState::fail(WorkItem* item){
    transition(item, new FailedState());
    return true;
}
WorkItemState* InProgressState::clone(){
        return new InProgressState(*this);
       }

 std::string InProgressState::getName(){
   return "In_Progress";
 }