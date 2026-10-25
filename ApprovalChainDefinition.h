/**
 * @file ApprovalChainDefinition.h
 * @brief Defines the ApprovalChainDefinition class.
 * 
 * Holds an ordered list of ApprovalStepDefinition objects. 
 * Represents the entire approval process (e.g., Team Lead -> Finance -> CTO) 
 * for a specific activity within a workflow.
 */

#ifndef APPROVAL_CHAIN_DEFINITION_H
#define APPROVAL_CHAIN_DEFINITION_H

#include <string>
#include <vector>
#include "ApprovalStepDefinition.h"

/**
 * @class ApprovalChainDefinition
 * @brief Represents a sequence of approval steps required for a specific task.
 * 
 * Used by the WorkflowEngine to build a dynamic Chain of Responsibility 
 * (ApprovalHandler) at runtime.
 */
class ApprovalChainDefinition {
private:
    std::string chainId;                       ///< Unique identifier for this approval chain.
    std::vector<ApprovalStepDefinition> steps; ///< The ordered list of approval steps.

public:
    /**
     * @brief Constructs a new ApprovalChainDefinition.
     * 
     * @param chainId The unique identifier for this approval chain.
     */
    ApprovalChainDefinition(const std::string& chainId);

    /**
     * @brief Appends a new step to the end of the approval chain.
     * 
     * @param step The ApprovalStepDefinition to add to the sequence.
     */
    void addStep(const ApprovalStepDefinition& step);
    
    /**
     * @brief Retrieves the unique identifier of this approval chain.
     * 
     * @return The chain ID as a string.
     */
    std::string getChainId() const;

    /**
     * @brief Retrieves the ordered list of approval steps.
     * 
     * @return A constant reference to the vector of ApprovalStepDefinition objects.
     */
    const std::vector<ApprovalStepDefinition>& getSteps() const;
};

#endif // APPROVAL_CHAIN_DEFINITION_H