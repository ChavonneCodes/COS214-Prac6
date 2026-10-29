#ifndef APPROVALHANDLER_H
#define APPROVALHANDLER_H

#include "Request.h"
#include <string>

enum class HandlerResult{
    Continue,
    Handled,
    Rejected
};

class ApprovalHandler {
public:
    virtual ~ApprovalHandler() = default;
    /**
     * @brief Returns the name of the approval handler.
     *
     * Provides the default name for the base ApprovalHandler.
     * Derived handler classes can override this method to return
     * their own names.
     *
     * @return The name of the handler.
     */
    virtual std::string getName() const;

    /**
     * @brief Sets the next handler in the chain.
     *
     * Stores a pointer to the next handler that should receive
     * a request when the current handler returns
     * HandlerResult::Continue.
     *
     * @param handler Pointer to the next handler in the chain.
     */
    void setNext(ApprovalHandler* handler);

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
    HandlerResult authorise(Request& request);

protected:
    // Each derived handler defines its own approval logic.
     /**
     * @brief Attempts to handle a request using the base handler.
     *
     * The base implementation returns HandlerResult::Continue,
     * allowing the request to pass to the next handler.
     *
     * @return HandlerResult::Continue by default.
     */
    virtual HandlerResult handleRequest(Request& request);

private:
    ApprovalHandler* next = nullptr;
};

#endif
