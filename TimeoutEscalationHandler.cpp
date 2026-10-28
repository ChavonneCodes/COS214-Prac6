#include "TimeoutEscalationHandler.h"
#include <iostream>

/**
 * @brief Constructs a TimeoutEscalationHandler without a work item.
 *
 * Initialises the work item pointer to nullptr. If a timeout
 * request reaches this handler, it will be rejected because
 * no work item is available for escalation.
 */
TimeoutEscalationHandler::TimeoutEscalationHandler(): workItem(nullptr){
}

/**
 * @brief Constructs a TimeoutEscalationHandler for a work item.
 *
 * Stores a pointer to the work item that this handler will
 * attempt to escalate when a matching timeout request is received.
 *
 * @param item Pointer to the work item to be escalated.
 */
TimeoutEscalationHandler::TimeoutEscalationHandler(WorkItem* item)
    : workItem(item) {}

/**
 * @brief Returns the name of this handler.
 *
 * Provides a name that identifies this handler in the
 * approval chain.
 *
 * @return The name "TimeoutEscalationHandler".
 */
std::string TimeoutEscalationHandler::getName() const {
    return "TimeoutEscalationHandler";
}

/**
 * @brief Processes a timeout request by attempting to escalate a work item.
 *
 * The handler first checks whether the request action is "TIMEOUT".
 * Requests with other actions are passed to the next handler.
 *
 * If no work item has been supplied, the request is rejected.
 * If the request refers to a different work item, the handler
 * returns HandlerResult::Continue.
 *
 * Work items in the Completed, Rejected, Cancelled, or Failed
 * states cannot be escalated. For other states, the handler
 * delegates the state transition to WorkItem::escalated().
 *
 * If escalation succeeds, the request's current state is updated
 * to "Escalated", and the handler returns HandlerResult::Handled.
 * If escalation is not permitted, the handler returns
 * HandlerResult::Rejected.
 *
 * @param request Reference to the timeout request being processed.
 * @return HandlerResult::Continue if the request is not a timeout
 *         or refers to a different work item;
 *         HandlerResult::Handled if escalation succeeds;
 *         HandlerResult::Rejected if escalation cannot proceed.
 */
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