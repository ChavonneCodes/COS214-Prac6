#ifndef WORKFLOWSTATEAPPROVALHANDLER_H
#define WORKFLOWSTATEAPPROVALHANDLER_H

#include "ApprovalHandler.h"

class WorkflowStateApprovalHandler : public ApprovalHandler {
public:
    /**
     * @brief Returns the name of this handler.
     *
     * Provides a name used to identify the handler in the
     * approval chain.
     *
     * @return The name "WorkflowStateApprovalHandler".
     */
    std::string getName() const override;

protected:
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
    HandlerResult handleRequest(Request& request) override;
};

#endif