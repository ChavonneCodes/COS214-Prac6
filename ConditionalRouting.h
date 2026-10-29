/**
 * @file ConditionalRouting.h
 * @brief Routing that branches on the outcome of the finished activity.
 *
 * Part of the **Strategy** pattern (role: ConcreteStrategy).
 */
#ifndef CONDITIONAL_ROUTING_H
#define CONDITIONAL_ROUTING_H

#include "RoutingStrategy.h"
#include "RoutingHelper.h"
/**
 * @class ConditionalRouting
 * @brief Goes to one activity if the outcome matches a condition, otherwise to another.
 */
class ConditionalRouting : public RoutingStrategy {
private:
    std::string condition;   ///< Outcome that counts as "true", e.g. "passed".
    std::string trueTarget;  ///< Id of the activity to go to when the outcome matches.
    std::string falseTarget; ///< Id of the activity to go to otherwise.

public:
    /**
     * @brief Creates a branching rule.
     * @param condition   Outcome that counts as "true".
     * @param trueTarget  Activity id used when the outcome equals the condition.
     * @param falseTarget Activity id used otherwise.
     */
    ConditionalRouting(const std::string& condition,
                       const std::string& trueTarget,
                       const std::string& falseTarget);

    /**
     * @brief Chooses the true or false target depending on the outcome.
     * @param completed  The activity that has just finished (unused).
     * @param definition The workflow definition used to look the target up.
     * @param outcome    Result of the finished activity.
     * @return A list with the chosen activity, or an empty list if it does not exist.
     */
    std::vector<const WorkflowComponent*> next(
        const WorkflowComponent& completed,
        const WorkflowDefinition& definition,
        const std::string& outcome) override;

    /**
     * @brief Returns "Conditional(" followed by the condition and ")".
     * @return "Conditional(" followed by the condition and ")".
     */
    std::string getName() const override;
};

#endif // CONDITIONAL_ROUTING_H
