#ifndef WORK_ITEM_H
#define WORK_ITEM_H

#include "RuntimeComponent.h"
#include "Participant.h"   // Strategy: the person an AssignmentStrategy picks
#include "ActivityDefinition.h"
#include "WorkItemState.h"
#include "EventBus.h"      // Observer: WorkItem publishes events to the bus
#include "WorkItemMemento.h"
#include "Data.h"


class WorkItem : public RuntimeComponent {
private:
    std::string instanceId;
    ActivityDefinition* definition;
    Participant* assignedParticipant = nullptr;  ///< Who the item is assigned to (not owned, may be null).
    std::string state;
    WorkItemState* currentState;
    EventBus* eventBus = nullptr;  ///< Bus that events are published to (not owned, may be null).
    Data data;

    /**
     * @brief Publishes an event on the bus; does nothing if no bus is attached.
     * @param eventName Kind of event, e.g. "STATE_CHANGED".
     * @param payload   Detail text describing what changed.
     */
    void publish(const std::string& eventName, const std::string& payload);

public:
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
    
    void restoreMemento(WorkItemMemento*);
    WorkItemMemento createMemento();
    std::string getId() const override;
    std::string getState() const override;
    void setState(const std::string& newState) override;
    void print(int indentLevel = 0) const override;
    void setParticipant(Participant* p);
    WorkItemState* getCurrentState();
    void changeState(WorkItemState* newState);
    bool assign(Participant* p);
    bool start();
    bool complete();
    bool reject();
    bool cancel() ;
    bool escalated();
    bool fail();
    bool makeAvailable();
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

};

#endif
