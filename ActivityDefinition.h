/**
 * @file ActivityDefinition.h
 * @brief Defines the ActivityDefinition class.
 * 
 * Part of the Composite pattern: this represents a Leaf node in the activity tree.
 * It represents a single piece of work (an activity) within the workflow blueprint.
 */

#ifndef ACTIVITY_DEFINITION_H
#define ACTIVITY_DEFINITION_H

#include "WorkflowComponent.h"
#include <iostream>
#include <string>

/**
 * @class ActivityDefinition
 * @brief Represents a single activity (task) in the workflow definition.
 * 
 * This class acts as a Leaf in the Composite pattern. It holds the step's ID, 
 * human-readable name, the ID of the role required to perform it, and an optional 
 * ID for an associated approval chain.
 */
class ActivityDefinition : public WorkflowComponent {
private:
    std::string activityId;      
    std::string name;            
    std::string requiredRoleId;   
    std::string approvalChainId;  

public:
    /**
     * @brief Constructs a new ActivityDefinition.
     * 
     * @param id The unique identifier for the activity.
     * @param name The human-readable name of the activity.
     * @param roleId The ID of the role required to perform this activity.
     * @param chainId The ID of the approval chain (defaults to an empty string if not applicable).
     */
    ActivityDefinition(const std::string& id, const std::string& name, 
                       const std::string& roleId, const std::string& chainId = "");

    /**
     * @brief Retrieves the ID of the approval chain associated with this activity.
     * 
     * @return The approval chain ID as a string (empty if none is assigned).
     */
    std::string getApprovalChainId() const;

    /**
     * @brief Prints the details of the activity to standard output.
     * 
     * Useful for debugging or visually displaying the Composite tree structure.
     * 
     * @param indentLevel The number of indentation levels to apply (2 spaces per level).
     */
    void print(int indentLevel) const override;

    /**
     * @brief Retrieves the unique identifier of the activity.
     * 
     * Overrides the pure virtual method in WorkflowComponent.
     * 
     * @return The activity ID.
     */
    std::string getId() const override;

    /**
     * @brief Retrieves the human-readable name of the activity.
     * 
     * Overrides the pure virtual method in WorkflowComponent.
     * 
     * @return The activity name.
     */
    std::string getName() const override;

    /**
     * @brief Retrieves the ID of the role required to perform this activity.
     * 
     * Used by the AssignmentStrategy to determine which participant should 
     * be assigned to the resulting WorkItem.
     * 
     * @return The required role ID.
     */
    std::string getRequiredRoleId() const;
};

#endif // ACTIVITY_DEFINITION_H