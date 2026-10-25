#ifndef WORK_ITEM_H
#define WORK_ITEM_H

#include "RuntimeComponent.h"
#include "Participant.h"   // Strategy: the person an AssignmentStrategy picks
#include "ActivityDefinition.h"
#include "WorkItemState.h"
#include "EventBus.h"      // Observer: WorkItem publishes events to the bus

class WorkItem : public RuntimeComponent {
private:
    std::string instanceId;
    ActivityDefinition* definition;
    Participant* assignedParticipant = nullptr;  ///< Who the item is assigned to (not owned, may be null).
    WorkItemState* currentState;
    EventBus* eventBus = nullptr;  ///< Bus that events are published to (not owned, may be null).

    /**
     * @brief Publishes an event on the bus; does nothing if no bus is attached.
     * @param eventName Kind of event, e.g. "STATE_CHANGED".
     * @param payload   Detail text describing what changed.
     */
    void publish(const std::string& eventName, const std::string& payload);

public:
    /**
     * @brief Constructs a new WorkItem.
     * 
     * @param instanceId The ID of the parent workflow instance.
     * @param def Pointer to the ActivityDefinition blueprint this item is based on.
     */
    WorkItem(const std::string& instanceId, ActivityDefinition* def);

    /**
     * @brief Returns the id of the role needed to perform this work item (used by AssignmentStrategy).
     * @return The required role id.
     */
    std::string getRequiredRoleId() const;

    /**
     * @brief Returns the participant this item is assigned to.
     * @return The assigned participant, or nullptr if unassigned.
     */
    Participant* getAssignedParticipant() const;

    /**
     * @brief Assigns the item to a participant and transitions its state to Assigned.
     *
     * If the item was already assigned, the previous participant loses one
     * open item and the new participant gains one.
     * @param participant Who to assign the item to (not owned).
     * @return true on success, false (and no change) if participant is nullptr.
     */
    bool assignTo(Participant* participant);
    
    /**
     * @brief Retrieves the unique identifier of the work item.
     * @return The activity ID.
     */
    std::string getId() const override;

    /**
     * @brief Retrieves the current state of the work item.
     * @return The current state name.
     */
    std::string getState() const override;

    /**
     * @brief Sets the state using a string (compatibility layer for tests/Observer).
     * @param newState The new state string.
     */
    void setState(const std::string& newState) override;

    /**
     * @brief Prints the work item details to standard output.
     * @param indentLevel The indentation level for hierarchical display.
     */
    void print(int indentLevel = 0) const override;

    /**
     * @brief Sets the assigned participant directly and updates their workload.
     * @param p Pointer to the participant.
     */
    void setParticipant(Participant* p);

    /**
     * @brief Retrieves the current state object.
     * @return Pointer to the current WorkItemState.
     */
    WorkItemState* getCurrentState();

    /**
     * @brief Transitions the work item to a new state and publishes an event.
     * @param newState Pointer to the new WorkItemState.
     */
    void changeState(WorkItemState* newState);

    bool assign(Participant* p);
    bool start();
    bool complete();
    bool reject();
    bool cancel();
    bool escalated();
    bool fail();
    bool makeAvailable();
    
    virtual ~WorkItem();

    /**
     * @brief Attaches the bus that state-change events are published to.
     * @param bus Event bus (not owned). Pass nullptr to stop publishing.
     */
    void setEventBus(EventBus* bus);

    /**
     * @brief Returns the id of the workflow instance this work item belongs to.
     * @return The instance id.
     */
    std::string getInstanceId() const;
};

#endif