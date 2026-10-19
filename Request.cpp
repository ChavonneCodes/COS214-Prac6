#include "Request.h"
using namespace std;

Request::Request(string action,string workItemId,string instanceId,string requiredRoleId,Data& d1) : action(action) , WorkItemId(workItemId) , instanceId(instanceId), requiredRoleId(requiredRoleId) , data(d1) {
}

string Request::getAction(){
    return action;
}

string Request::getRequiredRoleId(){
    return requiredRoleId;
}