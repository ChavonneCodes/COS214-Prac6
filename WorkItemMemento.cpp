#include "WorkItemMemento.h"
using namespace std;

WorkItemMemento::WorkItemMemento() : activityId("") , workItemId("") , state(nullptr),participant(nullptr) , dataSnapshot(""){
}

WorkItemMemento::WorkItemMemento(std::string activityId,std::string workItemId,Data& snapshot,WorkItemState* state,Participant* participant) : activityId(activityId) , workItemId(workItemId) , state(state != nullptr ? state->clone() : nullptr),participant(participant), dataSnapshot(snapshot){
}

WorkItemMemento::WorkItemMemento(const WorkItemMemento& other)
    : activityId(other.activityId),
      workItemId(other.workItemId),
      state(other.state != nullptr ? other.state->clone() : nullptr),
      participant(other.participant),
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
        participant = other.participant;
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
Participant* WorkItemMemento:: getCurrentParticipant()const{
    return participant;
}