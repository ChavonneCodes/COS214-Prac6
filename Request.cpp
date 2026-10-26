#include "Request.h"
using namespace std;

Request::Request(const Data& data): data(data){
}

Request::Request(const string& requestId,
            const string& action,
            const string& workItemId,
            const string& instanceId,
            const string& requiredRoleId,
            const string& requesterId,
            const string& reason,
            const string& currentState,
            const Data& data)
    : requestId(requestId),action(action), workItemId(workItemId), instanceId(instanceId),
      requiredRoleId(requiredRoleId),requesterId(requesterId) , reason(reason), currentState(currentState),data(data)
{
}

string Request::getAction() const {
    return action;
}

string Request::getRequiredRoleId() const {
    return requiredRoleId;
}

string Request::getWorkItemId() const{
    return workItemId;
}
string Request::getInstanceId() const{
    return instanceId;
}

Data Request::getData() const{
    return data;
}

string Request::getRequesterId() const{
    return requesterId;
}

string Request::getCurrentState() const{
    return currentState;
}

//setters
void Request::setAction(const string& action){
    this->action = action;
}

void Request::setCurrentState(const string& currentState){

}
void Request::setData(const Data& data){
    this->data = data;
}

void Request::setReason(const string& reason){
    this->reason = reason;
}

void Request::setRequiredRoleId(const string& requiredRoleId){
    this->requiredRoleId = requiredRoleId;
}

//validator
bool Request::isValid(){
    return !requestId.empty()
        && !action.empty()
        && !workItemId.empty()
        && !instanceId.empty()
        && !requiredRoleId.empty()
        && !requesterId.empty();
}