#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include <string>
#include <vector>
#include "RuntimeComponent.h"
#include "WorkItem.h"
#include "EventBus.h"
#include "Data.h"

/**
 * @brief One running execution of a WorkflowDefinition.
 *
 * Created by the WorkflowEngine (Facade). The instance stays pinned to the
 * definition version it was started with, so later versions never change it.
 * It owns the runtime tree (stages and work items) built for it.
 *
 * TODO: the instance lifecycle is a plain state name for now. UML V4 models it
 * with RunState objects (State pattern).
 */
class WorkflowInstance {
private:
    std::string instanceId;
    std::string definitionId;
    int definitionVersion;           ///< Version this instance is pinned to.
    RuntimeComponent* root;          ///< Runtime tree of stages and work items (owned).
    EventBus* eventBus;              ///< Bus that lifecycle events are published on (not owned, may be null).
    std::string state = "CREATED";   ///< CREATED, RUNNING, COMPLETED, FAILED or CANCELLED.
    Data data;                       ///< Case data, e.g. the proposal budget.

public:
    /**
     * @brief Creates an instance in the CREATED state.
     * @param instanceId        Unique id of this instance.
     * @param definitionId      Id of the definition it runs.
     * @param definitionVersion Version of the definition it is pinned to.
     * @param root              Runtime tree (ownership passes to the instance).
     * @param eventBus          Bus for lifecycle events (not owned, may be null).
     */
    WorkflowInstance(const std::string& instanceId, const std::string& definitionId,
                     int definitionVersion, RuntimeComponent* root, EventBus* eventBus);

    /** @brief Deletes the runtime tree. */
    ~WorkflowInstance();

    std::string getInstanceId() const;
    std::string getDefinitionId() const;
    int getDefinitionVersion() const;
    RuntimeComponent* getRoot() const;

    /**
     * @brief Returns every work item in the runtime tree, in tree order.
     * @return The work items (still owned by the instance).
     */
    std::vector<WorkItem*> getWorkItems() const;

    /**
     * @brief Finds the work item created for an activity.
     * @param activityId Id of the activity.
     * @return The work item, or nullptr if there is none.
     */
    WorkItem* findWorkItem(const std::string& activityId) const;

    std::string getState() const;

    /**
     * @brief Changes the lifecycle state and publishes INSTANCE_STATE_CHANGED.
     * @param newState Name of the new state, e.g. "RUNNING".
     */
    void setState(const std::string& newState);

    /**
     * @brief Returns the case data so it can be read or updated.
     * @return The case data.
     */
    Data& getData();

    WorkflowInstance(const WorkflowInstance&) = delete;
    WorkflowInstance& operator=(const WorkflowInstance&) = delete;
};

#endif
