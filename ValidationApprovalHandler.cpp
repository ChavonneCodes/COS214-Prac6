#include "ValidationApprovalHandler.h"
#include <iostream>

using namespace std;

string ValidationApprovalHandler::getName() const {
    return "ValidationApprovalHandler";
}

HandlerResult ValidationApprovalHandler::handleRequest(
    Request& request) {

    if (!request.isValid()) {
        cout << "Validation failed: request is incomplete.\n";
        return HandlerResult::Rejected;
    }

    cout << "Request validation passed.\n";
    return HandlerResult::Continue;
}