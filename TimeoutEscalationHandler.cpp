#include "TimeoutEscalationHandler.h"
#include <iostream>

using namespace std;

string TimeoutEscalationHandler::getName() const {
    return "TimeoutEscalationHandler";
}

HandlerResult TimeoutEscalationHandler::handleRequest(
    Request& request) {

    if (request.getAction() != "TIMEOUT") {
        return HandlerResult::Continue;
    }

    cout << "Timeout detected for work item: "
         << request.getWorkItemId() << '\n';

    cout << "Escalating request to a supervisor.\n";

    // TODO: Implement actual escalation, such as
    // updating the workflow or notifying a supervisor.

    return HandlerResult::Handled;
}