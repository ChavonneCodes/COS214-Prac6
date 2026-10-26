#include "RoleApprovalHandler.h"
#include <iostream>

using namespace std;

RoleApprovalHandler::RoleApprovalHandler(const string& roleId)
    : roleId(roleId) {
}

string RoleApprovalHandler::getName() const {
    return "RoleApprovalHandler";
}

HandlerResult RoleApprovalHandler::handleRequest(
    Request& request) {

    if (request.getAction() != "APPROVE") {
        return HandlerResult::Continue;
    }

    if (request.getRequiredRoleId() == roleId) {
        cout << "Required role matched: " << roleId << '\n';
        return HandlerResult::Handled;
    }

    cout << "This handler cannot process role: "
         << request.getRequiredRoleId() << '\n';

    return HandlerResult::Continue;
}