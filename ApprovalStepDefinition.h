/**
 * @file ApprovalStepDefinition.h
 * @brief Defines the ApprovalStepDefinition class.
 * 
 * Represents a single step in a multi-level approval chain. It defines who
 * approves at this step, how long they have to do it (timeout), and what happens if
 * they don't (onTimeout).
 */

#ifndef APPROVAL_STEP_DEFINITION_H
#define APPROVAL_STEP_DEFINITION_H

#include <string>

/**
 * @class ApprovalStepDefinition
 * @brief Represents one level of approval within an ApprovalChainDefinition.
 * 
 * This class holds the configuration for a single approver. When the WorkflowEngine
 * reaches this step, it will use these parameters to construct the corresponding
 * ApprovalHandler in the Chain of Responsibility.
 */
class ApprovalStepDefinition {
private:
    std::string roleId;      ///< The ID of the role required to approve this step.
    int timeoutSeconds;      ///< The time limit (in seconds) before an action is triggered.
    std::string onTimeout;   ///< The action to take if the timeout is reached ("escalate", "auto-approve", "auto-reject").

public:
    /**
     * @brief Constructs a new ApprovalStepDefinition.
     * 
     * @param roleId The ID of the role required to approve this step.
     * @param timeoutSeconds The time limit in seconds before the timeout action is triggered.
     * @param onTimeout The action to take upon timeout (e.g., "escalate", "auto-approve", "auto-reject").
     */
    ApprovalStepDefinition(const std::string& roleId, int timeoutSeconds, const std::string& onTimeout);
    
    /**
     * @brief Retrieves the ID of the role required to approve this step.
     * 
     * @return The required role ID as a string.
     */
    std::string getRoleId() const;

    /**
     * @brief Retrieves the timeout duration for this step.
     * 
     * @return The timeout in seconds.
     */
    int getTimeout() const;

    /**
     * @brief Retrieves the action to be taken if the timeout is reached.
     * 
     * @return The timeout action as a string (e.g., "escalate").
     */
    std::string getOnTimeout() const;
};

#endif // APPROVAL_STEP_DEFINITION_H