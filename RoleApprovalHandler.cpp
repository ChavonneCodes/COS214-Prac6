#include "RoleApprovalHandler.h"
using namespace std;


RoleApprovalHandler::RoleApprovalHandler(const string& roleId) : roleId(roleId){
}

string RoleApprovalHandler::getName(){
    return "Role Approval Handler";
}

// This handler is responsible for the required role.
bool RoleApprovalHandler::handleRequest(Request& request) {
    if (request.getRequiredRoleId() == roleId) {
        return true;
    }
    return false;
}