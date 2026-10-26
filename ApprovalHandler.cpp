
#include "ApprovalHandler.h"
#include <iostream>

using namespace std;

/**
 * @brief Returns the name of the approval handler.
 *
 * Provides the default name for the base ApprovalHandler.
 * Derived handler classes can override this method to return
 * their own names.
 *
 * @return The name of the handler.
 */
string ApprovalHandler::getName() const{
    return "Approval Handler";
}

/**
 * @brief Sets the next handler in the chain.
 *
 * Stores a pointer to the next handler that should receive
 * a request when the current handler returns
 * HandlerResult::Continue.
 *
 * @param handler Pointer to the next handler in the chain.
 */
void ApprovalHandler::setNext(ApprovalHandler* handler) {
    next = handler;
}

/**
 * @brief Attempts to handle a request using the base handler.
 *
 * The base implementation returns HandlerResult::Continue,
 * allowing the request to pass to the next handler.
 *
 * @return HandlerResult::Continue by default.
 */
HandlerResult ApprovalHandler::handleRequest(Request&) {
    // The base handler cannot approve requests itself.
    return HandlerResult::Continue;
}

/**
 * @brief Processes a request through the chain of responsibility.
 *
 * Calls handleRequest() to allow the current handler to process
 * the request. The returned result determines what happens next:
 *
 * - HandlerResult::Handled: The request was handled successfully,
 *   and processing stops.
 * - HandlerResult::Rejected: The request is rejected, and
 *   processing stops.
 * - HandlerResult::Continue: The request is passed to the next
 *   handler, if one exists.
 *
 * If the request must continue but no next handler is available,
 * the method reports that no handler could authorise the request
 * and returns HandlerResult::Rejected.
 *
 * @param request Reference to the request being processed.
 * @return The final result of processing the request.
 */
HandlerResult ApprovalHandler::authorise(Request& request) {

    HandlerResult result = handleRequest(request);
    // Step 1: Try to handle the request here.
    if (result == HandlerResult::Handled) {
        cout << getName() << " handled the request.\n";
        return result;
    }

    if(result == HandlerResult::Rejected){
        cout << getName() << " rejected the Request.\n";
        return result;
    }

    // Step 2: Pass the request to the next handler.
    if (next != nullptr) {
        return next->authorise(request);
    }

    // Step 3: No handler could handle the request.
    cout << "No handler could authorise the request.\n";
    return HandlerResult::Rejected;
}
