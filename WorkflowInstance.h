/**
 * @file WorkflowInstance.h
 * @brief Defines the WorkflowInstance class.
 * 
 * Represents a single, live running case of a workflow. It holds the reference 
 * to its specific blueprint (by ID and version) and owns the mutable Runtime 
 * Composite tree.
 */

#ifndef WORKFLOW_INSTANCE_H
#define WORKFLOW_INSTANCE_H

#include <string>
#include <vector>
#include "RuntimeComponent.h"
#include "EventBus.h"
#include "Data.h"
#include "WorkItem.h"

/**
 * @class WorkflowInstance
 * @brief Represents a single running execution of a workflow blueprint.
 * 
 * While a WorkflowDefinition is shared and read-only, a WorkflowInstance is unique 
 * to one specific case (e.g., "project-approval-1"). It maintains its own state, 
 * its own data payload (e.g., budget amounts), and its own dynamically growing 
 * Runtime Composite tree.
 * 
 * @note This class takes ownership of the runtime tree pointer (`root`). 
 * It is responsible for deleting the tree when the instance is destroyed.
 */
class WorkflowInstance {
private:
    std::string instanceId;                 ///< Unique ID for this specific running case.
    std::string definitionId;               ///< The ID of the blueprint this instance follows.
    int definitionVersion;                  ///< The specific version of the blueprint being used.
    RuntimeComponent* root;                 ///< The root of the runtime Composite tree (owned by the instance).
    EventBus* eventBus;                     ///< Pointer to the event bus for publishing events (not owned).
    std::string state;                      ///< The overall state of the workflow instance (e.g., "RUNNING", "COMPLETED").
    Data data;                              ///< The data payload for this instance (e.g., budget, applicant details).

public:
    /**
     * @brief Constructs a new WorkflowInstance.
     * 
     * @param id The unique identifier for this instance.
     * @param defId The ID of the workflow definition being executed.
     * @param version The version of the workflow definition being executed.
     * @param rootTree Pointer to the already-instantiated Runtime tree. The instance takes ownership.
     * @param bus Pointer to the EventBus for publishing state changes.
     */
    WorkflowInstance(const std::string& id, const std::string& defId, int version, 
                     RuntimeComponent* rootTree, EventBus* bus);

    /**
     * @brief Destructor that recursively deletes the runtime Composite tree.
     */
    ~WorkflowInstance();

    /**
     * @brief Retrieves the unique ID of this instance.
     * @return The instance ID.
     */
    std::string getId() const;

    /**
     * @brief Retrieves the ID of the blueprint this instance is based on.
     * @return The definition ID.
     */
    std::string getDefinitionId() const;

    /**
     * @brief Retrieves the version of the blueprint this instance is pinned to.
     * @return The definition version number.
     */
    int getDefinitionVersion() const;
    
    /**
     * @brief Retrieves the overall state of the workflow instance.
     * @return The state string (e.g., "RUNNING").
     */
    std::string getState() const;

    /**
     * @brief Sets the overall state of the workflow instance.
     * @param newState The new state string.
     */
    void setState(const std::string& newState);

    /**
     * @brief Retrieves the root of the runtime Composite tree.
     * @return Pointer to the root RuntimeComponent.
     */
    RuntimeComponent* getRoot() const;
    
    /**
     * @brief Retrieves the mutable data payload for this instance.
     * @return A reference to the Data object.
     */
    Data& getData(); 

    /**
     * @brief Recursively traverses the runtime tree and collects all WorkItems.
     * 
     * @return A vector of pointers to all WorkItems currently in the tree.
     */
    std::vector<WorkItem*> getWorkItems() const;

    /**
     * @brief Searches the runtime tree for a specific WorkItem by its ID.
     * 
     * @param workItemId The ID of the work item to find.
     * @return Pointer to the WorkItem, or nullptr if it does not exist in the tree.
     */
    WorkItem* findWorkItem(const std::string& workItemId) const; 
};

#endif // WORKFLOW_INSTANCE_H