#include "ValidationApprovalHandler.h"
#include <iostream>

using namespace std;

/**
 * @brief Returns the name of this handler.
 *
 * Provides a descriptive name used to identify this handler
 * in the approval chain.
 *
 * @return The name "Validation Approval Handler".
 */
string ValidationApprovalHandler::getName() const {
    return "Validation Approval Handler";
}

/**
 * @brief Validates a request before allowing it to continue.
 *
 * Calls Request::isValid() to determine whether the request
 * contains the required information.
 *
 * If validation fails, an error message is displayed and the
 * request is rejected. If validation succeeds, a confirmation
 * message is displayed and the request is passed to the next
 * handler for further processing.
 *
 * @param request Reference to the request being validated.
 * @return HandlerResult::Rejected if the request is invalid;
 *         HandlerResult::Continue if validation succeeds.
 */
HandlerResult ValidationApprovalHandler::handleRequest(
    Request& request) {

    if (!request.isValid()) {
        cout << "Validation failed: request is incomplete.\n";
        return HandlerResult::Rejected;
    }

    cout << "Request validation passed.\n";
    return HandlerResult::Continue;
}