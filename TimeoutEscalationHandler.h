#ifndef TIMEOUTESCALATIONHANDLER_H
#define TIMEOUTESCALATIONHANDLER_H

#include "ApprovalHandler.h"
#include "WorkItem.h"

class TimeoutEscalationHandler : public ApprovalHandler {
public:
    /**
     * @brief Returns the name of this handler.
     *
     * Provides a name that identifies this handler in the
     * approval chain.
     *
     * @return The name "TimeoutEscalationHandler".
     */
    std::string getName() const override;

    /**
     * @brief Constructs a TimeoutEscalationHandler for a work item.
     *
     * Stores a pointer to the work item that this handler will
     * attempt to escalate when a matching timeout request is received.
     *
     * @param item Pointer to the work item to be escalated.
     */
    TimeoutEscalationHandler(WorkItem*);

    /**
     * @brief Constructs a TimeoutEscalationHandler without a work item.
     *
     * Initialises the work item pointer to nullptr. If a timeout
     * request reaches this handler, it will be rejected because
     * no work item is available for escalation.
     */
    TimeoutEscalationHandler();

protected:
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
    HandlerResult handleRequest(Request& request) override;

private:
    WorkItem* workItem;
};

#endif