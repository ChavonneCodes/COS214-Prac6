/**
 * @file WorkflowDefinition.h
 * @brief Defines the WorkflowDefinition class.
 * 
 * Part of the Builder pattern (role: Product). This is the final, immutable 
 * blueprint of a workflow. It is populated by the Builder before being handed 
 * over to the Workflow Engine or Repository.
 */

#ifndef WORKFLOW_DEFINITION_H
#define WORKFLOW_DEFINITION_H

#include <string>
#include <vector>
#include <stdexcept>
#include "ActivityDefinition.h"
#include "RoleDefinition.h"
#include "ApprovalChainDefinition.h"
#include "TransitionDefinition.h"
#include "WorkflowComponent.h" // Added for WorkflowComponent* root

/**
 * @class WorkflowDefinition
 * @brief The immutable blueprint of a workflow process.
 * 
 * This class acts as the final Product of the Builder pattern. It holds a 
 * flattened list of all roles, transitions, and approval chains, as well as 
 * the root pointer to the hierarchical Composite tree of stages and activities.
 * 
 * Once built, it is treated as read-only. Instances created from this definition 
 * reference it by ID and version.
 */
class WorkflowDefinition {
private:
    std::string definitionId;       ///< The unique identifier for the workflow.
    std::string name;               ///< The human-readable name of the workflow.
    int version;                    ///< The version number of this blueprint.
    std::string chainId;            ///< (Unused) The ID of the primary approval chain.

    std::vector<ActivityDefinition> activities;         ///< Flat list of all activities in the workflow.
    std::vector<RoleDefinition> roles;                  ///< Flat list of all roles used in the workflow.
    std::vector<ApprovalChainDefinition> approvalChains;///< Flat list of all approval chains.
    std::vector<TransitionDefinition> transitions;      ///< Flat list of all routing connections.

    WorkflowComponent* root = nullptr; ///< Root of the Composite tree (stages, activities, subprocesses). The definition owns this.

public:
    /**
     * @brief Constructs a new WorkflowDefinition.
     * 
     * @param id The unique identifier for the workflow.
     * @param name The human-readable name of the workflow.
     * @param version The version number.
     */
    WorkflowDefinition(const std::string& id, const std::string& name, int version);

    /**
     * @brief Destructor that recursively deletes the Composite tree.
     */
    ~WorkflowDefinition(); 

    // --- Rule of 5: Memory Management ---
    // Since this class owns a raw pointer (root), we must disable copying 
    // to prevent double-free errors, and implement move semantics to safely 
    // transfer ownership.

    WorkflowDefinition(const WorkflowDefinition&) = delete;            ///< Copy constructor disabled to prevent shallow copies.
    WorkflowDefinition& operator=(const WorkflowDefinition&) = delete; ///< Copy assignment disabled.

    /**
     * @brief Move constructor that safely transfers the root pointer.
     * @param other The WorkflowDefinition being moved.
     */
    WorkflowDefinition(WorkflowDefinition&& other) noexcept;

    /**
     * @brief Move assignment that safely transfers the root pointer.
     * @param other The WorkflowDefinition being moved.
     * @return A reference to this updated definition.
     */
    WorkflowDefinition& operator=(WorkflowDefinition&& other) noexcept;

    // --- Composite Tree Management ---

    /**
     * @brief Sets the root of the Composite tree. The definition takes ownership.
     * @param rootNode Pointer to the root WorkflowComponent.
     */
    void setRoot(WorkflowComponent* rootNode);

    /**
     * @brief Retrieves the root of the Composite tree.
     * @return Pointer to the root WorkflowComponent.
     */
    WorkflowComponent* getRoot() const;

    /**
     * @brief Creates an editable copy of this definition under a new version (Prototype pattern).
     *
     * The copy has the same definition id and name, copies of every role,
     * activity, transition and approval chain, and a deep copy of the
     * Composite tree (made with WorkflowComponent::clone()). Only the version
     * number differs. Editing the copy (setRoot, addChild on its tree, addRole,
     * ...) never changes this definition, so a team can tweak a workflow
     * without rebuilding it with the Builder.
     *
     * @note The caller owns the returned pointer (for example by passing it to
     *       InMemoryDefinitionRepository::save, which stores it).
     * @param newVersion The version of the copy. Must differ from getVersion().
     * @throws std::invalid_argument if newVersion equals this definition's version.
     * @return A newly allocated independent WorkflowDefinition.
     */
    WorkflowDefinition* cloneWithVersion(int newVersion) const;

    // --- Methods used by the Builder to populate the definition ---

    /**
     * @brief Adds a flat activity to the workflow definition.
     * @param activity The ActivityDefinition to add.
     */
    void addActivity(const ActivityDefinition& activity);

    /**
     * @brief Adds a role to the workflow definition.
     * @param role The RoleDefinition to add.
     */
    void addRole(const RoleDefinition& role);

    /**
     * @brief Adds an approval chain to the workflow definition.
     * @param approvalChain The ApprovalChainDefinition to add.
     */
    void addApprovalChain(const ApprovalChainDefinition& approvalChain);

    /**
     * @brief Adds a routing transition to the workflow definition.
     * @param transition The TransitionDefinition to add.
     */
    void addTransition(const TransitionDefinition& transition);

    // --- Accessors ---

    /**
     * @brief Retrieves the unique definition ID.
     * @return The definition ID.
     */
    std::string getDefinitionId() const;

    /**
     * @brief Retrieves the human-readable name.
     * @return The workflow name.
     */
    std::string getName() const;

    /**
     * @brief Retrieves the version number.
     * @return The version number.
     */
    int getVersion() const;

    /**
     * @brief Retrieves the flat list of activities.
     * @return A constant reference to the activities vector.
     */
    const std::vector<ActivityDefinition>& getActivities() const;

    /**
     * @brief Retrieves the flat list of roles.
     * @return A constant reference to the roles vector.
     */
    const std::vector<RoleDefinition>& getRoles() const;

    /**
     * @brief Retrieves the flat list of approval chains.
     * @return A constant reference to the approval chains vector.
     */
    const std::vector<ApprovalChainDefinition>& getApprovalChains() const;

    /**
     * @brief Retrieves the flat list of routing transitions.
     * @return A constant reference to the transitions vector.
     */
    const std::vector<TransitionDefinition>& getTransitions() const;

    /**
     * @brief Looks up an approval chain by its unique ID.
     * 
     * @param chainId The ID of the chain to find.
     * @return Pointer to the matching ApprovalChainDefinition, or nullptr if not found.
     */
    ApprovalChainDefinition* getApprovalChainById(const std::string& chainId);
};

#endif // WORKFLOW_DEFINITION_H