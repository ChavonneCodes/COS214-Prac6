#include "WorkflowStateApprovalHandler.h"
#include <iostream>

using namespace std;


string WorkflowStateApprovalHandler::getName() const {
    return "WorkflowStateApprovalHandler";
}

HandlerResult WorkflowStateApprovalHandler::handleRequest(
    Request& request) {

    // Let the timeout handler handle timeout requests.
    if (request.getAction() == "TIMEOUT") {
        return HandlerResult::Continue;
    }

    const string state = request.getCurrentState();

    if (state == "Created" || state == "PickedUp") {
        cout << "Workflow state permits approval.\n";
        return HandlerResult::Continue;
    }

    cout << "Approval rejected: state '" << state
         << "' does not permit approval.\n";

    return HandlerResult::Rejected;
}