#include "Request.h"
using namespace std;


Request::Request(string action, string workItemId, string instanceId,
                 string requiredRoleId, string value)
    : action(action), WorkItemId(workItemId), instanceId(instanceId),
      requiredRoleId(requiredRoleId), data(value)
{
}

string Request::getAction(){
    return action;
}

string Request::getRequiredRoleId(){
    return requiredRoleId;
}