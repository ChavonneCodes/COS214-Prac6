#include "WorkflowStateApprovalHandler.h"
#include <iostream>

using namespace std;

/**
 * @brief Returns the name of this handler.
 *
 * Provides a name used to identify the handler in the
 * approval chain.
 *
 * @return The name "WorkflowStateApprovalHandler".
 */
string WorkflowStateApprovalHandler::getName() const {
    return "Workflow State Approval Handler";
}

/**
 * @brief Checks whether the current workflow state permits approval.
 *
 * Timeout requests are allowed to continue through the chain
 * so that the timeout escalation handler can process them.
 *
 * For other requests, the handler checks the current workflow
 * state. Requests in the Created or PickedUp states are allowed
 * to continue to the next handler.
 *
 * Requests in any other state are rejected.
 *
 * @param request Reference to the request being processed.
 * @return HandlerResult::Continue if the action is TIMEOUT or
 *         the workflow state permits approval;
 *         HandlerResult::Rejected if the state does not permit approval.
 */
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