#include "WorkItemMemento.h"
using namespace std;

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