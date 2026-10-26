/**
 * @file WorkflowBlueprint.h
 * @brief Defines the WorkflowBlueprint data structure.
 * 
 * A simple data container used by the user/client to specify the raw 
 * configuration and structure of a workflow before it is built.
 */

#ifndef WORKFLOW_BLUEPRINT_H
#define WORKFLOW_BLUEPRINT_H

#include <string>
#include <vector>
#include "RoleDefinition.h"
#include "ActivityDefinition.h"
#include "TransitionDefinition.h"
#include "ApprovalChainDefinition.h"
#include "WorkflowComponent.h"
#include "WorkFlowNodeData.h"

/**
 * @struct WorkflowBlueprint
 * @brief A Data Transfer Object (DTO) carrying the raw configuration for a workflow.
 * 
 * This struct is populated by the client (e.g., from a TUI or a JSON file) and 
 * passed to the WorkflowDirector and ConcreteWorkflowBuilder. The Builder reads 
 * this data to instantiate the immutable WorkflowDefinition product, handling 
 * the complex memory management of the Composite tree.
 */
struct WorkflowBlueprint {
    std::string id;                         ///< The unique identifier for the workflow.
    std::string name;                       ///< The human-readable name of the workflow.
    int version = 1;                        ///< The version number (defaults to 1).
    
    std::vector<RoleDefinition> roles;          ///< Flat list of roles used in the workflow.
    std::vector<ActivityDefinition> activities; ///< Flat list of individual activities.
    std::vector<TransitionDefinition> transitions; ///< Flat list of routing connections between activities.
    std::vector<ApprovalChainDefinition> approvalChains; ///< Flat list of multi-level approval chains.
    
    /**
     * @brief The raw data for the Composite tree structure.
     * 
     * This represents the hierarchical layout (Stages containing Activities/SubProcesses).
     * It is optional, as a user might choose to build a flat workflow using only 
     * the 'activities' vector above.
     */
    WorkflowNodeData rootTreeData; 
    
    /**
     * @brief Flag indicating whether rootTreeData has been populated.
     * 
     * Used by the Builder to determine if it needs to instantiate a Composite tree
     * or just use the flat activity list.
     */
    bool hasRootTree = false;
};

#endif // WORKFLOW_BLUEPRINT_H