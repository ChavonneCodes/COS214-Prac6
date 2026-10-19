/**
 * @file RoutingStrategy.h
 * @brief Abstract rule for deciding which activity(ies) come after a finished one.
 *
 * Part of the **Strategy** pattern (role: Strategy).
 */
#ifndef ROUTING_STRATEGY_H
#define ROUTING_STRATEGY_H

#include <string>
#include <vector>
#include "ActivityDefinition.h"
#include "WorkflowDefinition.h"

/**
 * @class RoutingStrategy
 * @brief Decides what happens next in a workflow once an activity is complete.
 *
 * Different workflows route differently (always the next step, or a branch
 * depending on the outcome). Each rule is a subclass, so the workflow code
 * never changes when the routing rule does.
 */
class RoutingStrategy {
public:
    virtual ~RoutingStrategy() = default;

    /**
     * @brief Finds the activities that follow a completed activity.
     * @param completed  The activity that has just finished.
     * @param definition The workflow definition holding the activities and transitions.
     * @param outcome    Result of the finished activity, e.g. "passed" or "failed".
     * @return The next activities (empty if the workflow ends here).
     */
    virtual std::vector<const ActivityDefinition*> next(
        const ActivityDefinition& completed,
        const WorkflowDefinition& definition,
        const std::string& outcome) = 0;

    /**
     * @brief Returns a short name describing the strategy.
     * @return A short name describing the strategy.
     */
    virtual std::string getName() const = 0;
};

#endif // ROUTING_STRATEGY_H
