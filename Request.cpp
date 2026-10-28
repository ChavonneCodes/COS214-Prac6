#include "Request.h"
using namespace std;

Request::Request(const Data& data)
    : action(""),
      workItemId(""),
      instanceId(""),
      requiredRoleId(""),
      data(data),
      requestId(""),
      requesterId(""),
      reason(""),
      currentState("")
{
}

Request::Request(
    const std::string& requestId,
    const std::string& action,
    const std::string& workItemId,
    const std::string& instanceId,
    const std::string& requiredRoleId,
    const std::string& requesterId,
    const std::string& reason,
    const std::string& currentState,
    const Data& data
)
    : action(action),
      workItemId(workItemId),
      instanceId(instanceId),
      requiredRoleId(requiredRoleId),
      data(data),
      requestId(requestId),
      requesterId(requesterId),
      reason(reason),
      currentState(currentState)
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
    this->currentState = currentState;
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