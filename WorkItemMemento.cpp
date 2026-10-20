#include "WorkItemMemento.h"
using namespace std;

WorkItemMemento::WorkItemMemento(string activityId,string workItemId,Data& snapshot,workItemState* state) : activityId(activityId) , workItemId(workItemId) , state(state) , dataSnapshot(data){
}

string WorkItemMemento::getWorkItemId(){
    return workItemId;
}

string WorkItemMemento::getActivityId(){
    return activityId;
}

WorkItemState* WorkItemMemento::getStateKind(){
    return state;
}

Data WorkItemMemento::getDataSnapshot(){
    return dataSnapshot;
}