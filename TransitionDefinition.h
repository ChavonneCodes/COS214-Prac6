/**
 * @file TransitionDefinition.h
 * @brief Defines the TransitionDefinition class.
 * 
 * Represents a connection (edge) between two activities. It is not a node 
 * in the Composite tree; it is the rule that says "after Activity A, go to Activity B".
 */

#ifndef TRANSITION_DEFINITION_H
#define TRANSITION_DEFINITION_H

#include <string>

/**
 * @class TransitionDefinition
 * @brief Represents a routing rule (edge) between two activities in a workflow.
 * 
 * While the Composite tree defines the structure (what stages and activities exist),
 * the TransitionDefinition defines the flow (what order they execute in). 
 * It is evaluated by the RoutingStrategy at runtime to determine the next step.
 */
class TransitionDefinition {
    private:
        std::string fromActivityId; ///< The ID of the source activity.
        std::string toActivityId;   ///< The ID of the destination activity.
        std::string condition;      ///< The guard condition for the transition (e.g., "always", "passed", "failed").

    public:
        /**
         * @brief Constructs a new TransitionDefinition.
         * 
         * @param from The ID of the activity that triggers this transition.
         * @param to The ID of the activity that becomes active if the condition is met.
         * @param condition The guard condition required to take this path.
         */
        TransitionDefinition(const std::string& from, const std::string& to, const std::string& condition);
        
        /**
         * @brief Retrieves the ID of the source activity.
         * 
         * @return The source activity ID as a string.
         */
        std::string getFrom() const;
        
        /**
         * @brief Retrieves the ID of the destination activity.
         * 
         * @return The destination activity ID as a string.
         */
        std::string getTo() const;
        
        /**
         * @brief Retrieves the guard condition for this transition.
         * 
         * @return The condition string (e.g., "always").
         */
        std::string getCondition() const;
};

#endif // TRANSITION_DEFINITION_H