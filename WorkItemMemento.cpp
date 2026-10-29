#include "WorkItemMemento.h"
using namespace std;

WorkItemMemento::WorkItemMemento() : activityId("") , workItemId("") , state(nullptr) , dataSnapshot(""){
}

WorkItemMemento::WorkItemMemento(string activityId,string workItemId,Data& snapshot,WorkItemState* state) : activityId(activityId) , workItemId(workItemId) , state(state != nullptr ? state->clone() : nullptr) , dataSnapshot(snapshot){
}

WorkItemMemento::WorkItemMemento(const WorkItemMemento& other)
    : activityId(other.activityId),
      workItemId(other.workItemId),
      state(other.state != nullptr ? other.state->clone() : nullptr),
      dataSnapshot(other.dataSnapshot)
{
}

WorkItemMemento& WorkItemMemento::operator=(const WorkItemMemento& other){
    if(this != &other){
        WorkItemState* copiedState = other.state != nullptr ? other.state->clone() : nullptr;

        delete state;

        activityId = other.activityId;
        workItemId = other.workItemId;
        dataSnapshot = other.dataSnapshot;
        state = copiedState;
    }
    
    return *this;
}

WorkItemMemento::~WorkItemMemento(){
    delete state;
}

string WorkItemMemento::getWorkItemId() const{
    return workItemId;
}

string WorkItemMemento::getActivityId() const{
    return activityId;
}

WorkItemState* WorkItemMemento::getStateKind() const{
    return state;
}

Data WorkItemMemento::getDataSnapshot() const{
    return dataSnapshot;
}