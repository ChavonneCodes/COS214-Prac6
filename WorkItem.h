#ifndef WORK_ITEM_H
#define WORK_ITEM_H

#include "RuntimeComponent.h"
#include "ActivityDefinition.h"
#include "EventBus.h"      // Observer: WorkItem publishes events to the bus

class WorkItem : public RuntimeComponent {
private:
    std::string instanceId;
    ActivityDefinition* definition;
    std::string state;
    EventBus* eventBus = nullptr;  ///< Bus that events are published to (not owned, may be null).

    /**
     * @brief Publishes an event on the bus; does nothing if no bus is attached.
     * @param eventName Kind of event, e.g. "STATE_CHANGED".
     * @param payload   Detail text describing what changed.
     */
    void publish(const std::string& eventName, const std::string& payload);

public:
    WorkItem(const std::string& instanceId, ActivityDefinition* def);
    
    std::string getId() const override;
    std::string getState() const override;
    void setState(const std::string& newState) override;
    void print(int indentLevel = 0) const override;

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