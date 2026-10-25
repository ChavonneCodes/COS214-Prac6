/**
 * @file ConcreteWorkflowBuilder.h
 * @brief Defines the ConcreteWorkflowBuilder class.
 * 
 * This class implements the abstract WorkflowBuilder interface. It accumulates
 * the flat workflow data and the raw composite tree data, and then assembles
 * them into a final, immutable WorkflowDefinition product.
 */

#ifndef CONCRETE_WORKFLOW_BUILDER_H
#define CONCRETE_WORKFLOW_BUILDER_H

#include "WorkflowBuilder.h"
#include "WorkFlowNodeData.h"
#include <vector>

/**
 * @class ConcreteWorkflowBuilder
 * @brief The Concrete Builder in the Builder pattern.
 * 
 * It provides a fluent interface (returning `*this`) to allow the Director 
 * or Client to construct a workflow step-by-step. When build() is called, 
 * it recursively instantiates the Composite tree from the raw WorkflowNodeData 
 * and packages it together with the flat lists (roles, transitions, etc.) 
 * into a WorkflowDefinition.
 */
class ConcreteWorkflowBuilder : public WorkflowBuilder {
private:
    std::string definitionId;       ///< The unique identifier for the workflow.
    std::string name;               ///< The human-readable name of the workflow.
    int version;                    ///< The version number of the workflow.
    
    std::vector<ActivityDefinition> activities;         ///< Flat list of activities.
    std::vector<TransitionDefinition> transitions;      ///< Flat list of routing transitions.
    std::vector<RoleDefinition> roles;                  ///< Flat list of roles.
    std::vector<ApprovalChainDefinition> approvalChains;///< Flat list of approval chains.
    
    WorkflowNodeData rootTreeData;  ///< Raw data used to build the Composite tree.
    bool hasRoot = false;           ///< Flag indicating if composite tree data was provided.

    /**
     * @brief Recursive helper function that builds the Composite tree.
     * 
     * Inspects the type of the provided WorkflowNodeData and instantiates the 
     * corresponding C++ object (StageDefinition, ActivityDefinition, or SubProcessDefinition).
     * 
     * @param data The raw data node to convert into a workflow component.
     * @return A dynamically allocated WorkflowComponent pointer (the caller assumes ownership).
     */
    WorkflowComponent* createComponent(const WorkflowNodeData& data);

public:
    /**
     * @brief Constructs a new ConcreteWorkflowBuilder with default values.
     */
    ConcreteWorkflowBuilder();

    // --- Implementing the Abstract Builder steps ---

    /**
     * @brief Sets the definition ID for the workflow.
     * @param id The unique workflow ID.
     * @return A reference to this builder (for method chaining).
     */
    WorkflowBuilder& setId(const std::string& id) override;

    /**
     * @brief Sets the name for the workflow.
     * @param name The human-readable workflow name.
     * @return A reference to this builder (for method chaining).
     */
    WorkflowBuilder& setName(const std::string& name) override;

    /**
     * @brief Sets the version for the workflow.
     * @param version The version number.
     * @return A reference to this builder (for method chaining).
     */
    WorkflowBuilder& setVersion(int version) override;

    /**
     * @brief Adds a role to the workflow definition.
     * @param roleId The unique ID of the role.
     * @param roleName The name of the role.
     * @param level The permission level of the role.
     * @return A reference to this builder (for method chaining).
     */
    WorkflowBuilder& addRole(const std::string& roleId, const std::string& roleName, int level) override;

    /**
     * @brief Adds an activity to the flat list of activities.
     * @param actId The unique activity ID.
     * @param name The human-readable activity name.
     * @param roleId The ID of the role required to perform the activity.
     * @return A reference to this builder (for method chaining).
     */
    WorkflowBuilder& addActivity(const std::string& actId, const std::string& name, const std::string& roleId) override;

    /**
     * @brief Adds a routing transition between two activities.
     * @param from The ID of the source activity.
     * @param to The ID of the destination activity.
     * @param condition The guard condition (e.g., "always", "passed").
     * @return A reference to this builder (for method chaining).
     */
    WorkflowBuilder& addTransition(const std::string& from, const std::string& to, const std::string& condition) override;

    /**
     * @brief Adds an approval chain to the workflow definition.
     * @param chain The constructed ApprovalChainDefinition.
     * @return A reference to this builder (for method chaining).
     */
    WorkflowBuilder& addApprovalChain(const ApprovalChainDefinition& chain) override;

    /**
     * @brief Sets the raw data for the Composite tree structure.
     * 
     * This data will be used during build() to dynamically instantiate the 
     * nested Stages, Activities, and SubProcesses.
     * 
     * @param rootData The root WorkflowNodeData object.
     * @return A reference to this builder (for method chaining).
     */
    WorkflowBuilder& setRoot(const WorkflowNodeData& rootData) override;

    /**
     * @brief Constructs and returns the final WorkflowDefinition product.
     * 
     * This method validates the required fields, assembles the flat lists,
     * and triggers createComponent() to build the recursive Composite tree.
     * 
     * @return A fully constructed WorkflowDefinition object.
     * @throws std::runtime_error if required fields (ID, name, activities) are missing.
     */
    WorkflowDefinition build() override;
};

#endif // CONCRETE_WORKFLOW_BUILDER_H