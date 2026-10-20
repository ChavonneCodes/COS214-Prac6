#include "WorkItemMemento.h"
using namespace std;

WorkItemMemento::WorkItemMemento(string activityId,string workItemId,Data& snapshot,WorkItemState* state) : activityId(activityId) , workItemId(workItemId) , state(state) , dataSnapshot(snapshot){
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