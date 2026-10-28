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
     * @brief Assigns the item to a participant and sets its state to "ASSIGNED".
     *
     * If the item was already assigned, the previous participant loses one
     * open item and the new participant gains one.
     * @param participant Who to assign the item to (not owned).
     * @return true on success, false (and no change) if participant is nullptr.
     */
    bool assignTo(Participant* participant);
    /**
     * @brief Restores the data and state from a saved memento.
     *
     * The snapshot is checked to ensure it belongs to this work item
     * and its associated activity. The state is cloned before replacing
     * the current state.
     *
     * @param memento Pointer to the memento to restore.
     *        If nullptr, no action is performed.
     */
    void restoreMemento(WorkItemMemento* memento);
    /**
     * @brief Creates a snapshot of this work item's current data and state.
     *
     * The WorkItemMemento constructor clones the state pointer, so the
     * snapshot remains valid after the work item changes state.
     *
     * @return A memento representing the current snapshot.
     */
    WorkItemMemento createMemento();
    
    /**
     * @brief Retrieves the unique identifier of the work item.
     * @return The activity ID.
     */
    std::string getId() const override;

    /**
     * @brief Retrieves the current state of the work item as a string.
     * @return The current state string.
     */
    std::string getState() const override;

    /**
     * @brief Sets the state string directly.
     * @param newState The new state string.
     */
    void setState(const std::string& newState) override;

    /**
     * @brief Prints the work item details to standard output.
     * @param indentLevel The indentation level for hierarchical display.
     */
    void print(int indentLevel = 0) const override;

    /**
     * @brief Sets the assigned participant directly without triggering state changes or events.
     * @param p Pointer to the participant.
     */
    void setParticipant(Participant* p);

    /**
     * @brief Retrieves the current state object (State pattern).
     * @return Pointer to the current WorkItemState.
     */
    WorkItemState* getCurrentState();

    /**
     * @brief Transitions the work item to a new state (State pattern).
     * 
     * Deletes the old state object and replaces it with the new one.
     * @param newState Pointer to the new WorkItemState.
     */
    void changeState(WorkItemState* newState);

    /**
     * @brief Transitions the work item to the Assigned state.
     * @param p Pointer to the participant being assigned.
     * @return true if the transition was successful, false otherwise.
     */
    bool assign(Participant* p);

    /**
     * @brief Transitions the work item to the Started/Running state.
     * @return true if the transition was successful, false otherwise.
     */
    bool start();

    /**
     * @brief Transitions the work item to the Completed/Success state.
     * @return true if the transition was successful, false otherwise.
     */
    bool complete();

    /**
     * @brief Transitions the work item to the Rejected state.
     * @return true if the transition was successful, false otherwise.
     */
    bool reject();

    /**
     * @brief Transitions the work item to the Cancelled state.
     * @return true if the transition was successful, false otherwise.
     */
    bool cancel() ;

    /**
     * @brief Transitions the work item to the Escalated state.
     * @return true if the transition was successful, false otherwise.
     */
    bool escalated();

    /**
     * @brief Transitions the work item to the Failed state.
     * @return true if the transition was successful, false otherwise.
     */
    bool fail();

    /**
     * @brief Transitions the work item to the Available state.
     * @return true if the transition was successful, false otherwise.
     */
    bool makeAvailable();

    /**
     * @brief Destructor that cleans up the current state object.
     */
    virtual ~WorkItem();

    /**
     * @brief Attaches the bus that state-change events are published to (Observer pattern).
     * @param bus Event bus (not owned). Pass nullptr to stop publishing.
     */
    void setEventBus(EventBus* bus);

    /**
     * @brief Returns the id of the workflow instance this work item belongs to.
     * @return The instance id.
     */
    std::string getInstanceId() const;

    /**
     * @brief Returns the display name of the activity this item performs.
     * @return The activity name.
     */
    std::string getName() const;

    /**
     * @brief Calls the visitor's visitWorkItem() for this item (Visitor pattern).
     * @param visitor The operation to run.
     */
    void accept(RuntimeVisitor& visitor) override;

};

#endif
