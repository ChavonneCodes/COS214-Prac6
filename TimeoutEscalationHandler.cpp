#include "TimeoutEscalationHandler.h"
#include <iostream>


TimeoutEscalationHandler::TimeoutEscalationHandler(): workItem(nullptr){
}

TimeoutEscalationHandler::TimeoutEscalationHandler(WorkItem* item)
    : workItem(item) {}

std::string TimeoutEscalationHandler::getName() const {
    return "TimeoutEscalationHandler";
}

HandlerResult TimeoutEscalationHandler::handleRequest(Request& request) {

    // This handler only handles timeout requests.
    if (request.getAction() != "TIMEOUT") {
        return HandlerResult::Continue;
    }

    if (workItem == nullptr) {
        std::cout << "Escalation failed: no WorkItem was supplied.\n";
        return HandlerResult::Rejected;
    }

    // Ensure the request refers to the supplied work item.
    if (request.getWorkItemId() != workItem->getId()) {
        return HandlerResult::Continue;
    }

    // Do not escalate an item that has already reached a terminal state.
    const std::string state = workItem->getState();

    if (state == "Completed" ||
        state == "Rejected" ||
        state == "Cancelled" ||
        state == "Failed") {
        std::cout << "Cannot escalate work item "
                  << workItem->getId()
                  << " because it is in state " << state << ".\n";

        return HandlerResult::Rejected;
    }

    // Delegate the transition to the State pattern.
    if (!workItem->escalated()) {
        std::cout << "Escalation is not allowed from state "
                  << workItem->getState() << ".\n";

        return HandlerResult::Rejected;
    }

    request.setCurrentState("Escalated");

    std::cout << "Work item " << workItem->getId()
              << " was escalated successfully.\n";

    return HandlerResult::Handled;
}