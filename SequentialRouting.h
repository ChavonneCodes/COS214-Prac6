/**
 * @file SequentialRouting.h
 * @brief Routing that always follows the "always" transitions.
 *
 * Part of the **Strategy** pattern (role: ConcreteStrategy).
 */
#ifndef SEQUENTIAL_ROUTING_H
#define SEQUENTIAL_ROUTING_H

#include "RoutingStrategy.h"

/**
 * @class SequentialRouting
 * @brief Moves on to the activities linked by an "always" transition, ignoring the outcome.
 */
class SequentialRouting : public RoutingStrategy {
public:
    /**
     * @brief Returns the targets of all "always" transitions leaving the completed activity.
     * @param completed  The activity that has just finished.
     * @param definition The workflow definition holding the transitions.
     * @param outcome    Ignored.
     * @return The next activities, or an empty list if there are none.
     */
    std::vector<const ActivityDefinition*> next(
        const ActivityDefinition& completed,
        const WorkflowDefinition& definition,
        const std::string& outcome) override;

    /**
     * @brief Returns "Sequential".
     * @return "Sequential".
     */
    std::string getName() const override;
};

#endif // SEQUENTIAL_ROUTING_H
