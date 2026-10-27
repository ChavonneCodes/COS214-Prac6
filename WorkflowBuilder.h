/**
 * @file WorkflowBuilder.h
 * @brief Defines the WorkflowBuilder abstract class.
 * 
 * Part of the Builder pattern (role: Builder). This class declares the 
 * construction steps that all concrete builders must implement.
 */

#ifndef WORKFLOW_BUILDER_H
#define WORKFLOW_BUILDER_H

#include "WorkflowDefinition.h"
#include "WorkFlowNodeData.h"
#include <string>

/**
 * @class WorkflowBuilder
 * @brief The Abstract Builder interface for constructing a WorkflowDefinition.
 * 
 * This interface defines the steps required to build a workflow. It uses a 
 * fluent interface (returning a reference to the builder) so that the Director 
 * or Client can chain method calls together. 
 * 
 * Concrete builders (like ConcreteWorkflowBuilder) will implement these methods 
 * to assemble the final, immutable WorkflowDefinition product.
 */
class WorkflowBuilder {
public:
    /**
     * @brief Virtual destructor to ensure proper cleanup of derived builders.
     */
    virtual ~WorkflowBuilder() = default;

    // --- Setup methods (User provides the data) ---

    /**
     * @brief Sets the unique definition ID.
     * @param id The workflow definition ID.
     * @return A reference to this builder (for method chaining).
     */
    virtual WorkflowBuilder& setId(const std::string& id) = 0;

    /**
     * @brief Sets the human-readable name of the workflow.
     * @param name The workflow name.
     * @return A reference to this builder (for method chaining).
     */
    virtual WorkflowBuilder& setName(const std::string& name) = 0;

    /**
     * @brief Sets the version number of the workflow.
     * @param version The version number.
     * @return A reference to this builder (for method chaining).
     */
    virtual WorkflowBuilder& setVersion(int version) = 0;

    // --- Add methods (User chooses exactly what to add) ---

    /**
     * @brief Adds a role to the workflow.
     * @param roleId The unique ID of the role.
     * @param roleName The name of the role.
     * @param level The permission level (0=low, 1=medium, 2=high).
     * @return A reference to this builder (for method chaining).
     */
    virtual WorkflowBuilder& addRole(const std::string& roleId, const std::string& roleName, int level) = 0;

    /**
     * @brief Adds a flat activity to the workflow.
     * @param actId The unique ID of the activity.
     * @param name The name of the activity.
     * @param roleId The ID of the role required to perform the activity.
     * @return A reference to this builder (for method chaining).
     */
    virtual WorkflowBuilder& addActivity(const std::string& actId, const std::string& name, const std::string& roleId) = 0;

    /**
     * @brief Adds a routing transition between two activities.
     * @param from The ID of the source activity.
     * @param to The ID of the destination activity.
     * @param condition The guard condition (e.g., "always", "passed").
     * @return A reference to this builder (for method chaining).
     */
    virtual WorkflowBuilder& addTransition(const std::string& from, const std::string& to, const std::string& condition) = 0;

    /**
     * @brief Adds a multi-level approval chain to the workflow.
     * @param chain The constructed ApprovalChainDefinition.
     * @return A reference to this builder (for method chaining).
     */
    virtual WorkflowBuilder& addApprovalChain(const ApprovalChainDefinition& chain) = 0;

    /**
     * @brief Sets the raw data used to build the nested Composite tree structure.
     * @param rootData The root WorkflowNodeData object.
     * @return A reference to this builder (for method chaining).
     */
    virtual WorkflowBuilder& setRoot(const WorkflowNodeData& rootData) = 0;

    // --- Finalize and return the product ---

    /**
     * @brief Constructs and returns the final WorkflowDefinition product.
     * 
     * This method triggers the actual assembly of the object, validates 
     * required fields, and transfers ownership of the internal components 
     * to the new WorkflowDefinition.
     * 
     * @return The fully constructed WorkflowDefinition.
     */
    virtual WorkflowDefinition build() = 0;
};

#endif // WORKFLOW_BUILDER_H