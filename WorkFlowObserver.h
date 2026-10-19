/**
 * @file WorkFlowObserver.h
 * @brief Abstract observer interface for workflow events.
 *
 * Part of the **Observer** pattern (role: Observer).
 */
#ifndef WORKFLOW_OBSERVER_H
#define WORKFLOW_OBSERVER_H

#include "WorkFlowEvent.h"

/**
 * @class WorkFlowObserver
 * @brief Anything that wants to react to workflow events implements this interface.
 */
class WorkFlowObserver {
public:
    virtual ~WorkFlowObserver() = default;

    /**
     * @brief Called by the EventBus for every published event.
     * @param event The event that was published.
     */
    virtual void update(const WorkFlowEvent& event) = 0;
};

#endif // WORKFLOW_OBSERVER_H
