#include "TimeoutEscalationHandler.h"
#include <iostream>


TimeoutEscalationHandler::TimeoutEscalationHandler(): workItem(nullptr), engine(nullptr){
}

TimeoutEscalationHandler::TimeoutEscalationHandler(WorkItem* item,WorkflowEngine* e)
    : workItem(item),engine(e){}

std::string TimeoutEscalationHandler::getName() const {
    return "TimeoutEscalationHandler";
}

HandlerResult TimeoutEscalationHandler::handleRequest(Request& request) {

    // This handler only handles timeout requests.
    if (request.getAction() != "TIMEOUT") {
        return HandlerResult::Continue;
    }

    if (workItem == nullptr||engine==nullptr) {
        std::cout << "Escalation failed: no WorkItem or engine was supplied.\n";
        return HandlerResult::Rejected;
    }

    // Ensure the request refers to the supplied work item.
    if (request.getWorkItemId() != workItem->getId()) {
        return HandlerResult::Continue;
    }

    // Do not escalate an item that has already reached a terminal state.
     if (!engine->submit(new EscalateCommand(workItem))) {
        std::cout << "Escalation not allowed from state " << workItem->getState() << ".\n";
        return HandlerResult::Rejected;
    }

    request.setCurrentState("Escalated");

    std::cout << "Work item " << workItem->getId()
              << " was escalated successfully.\n";

    return HandlerResult::Handled;
}