/**
 * @file WorkFlowEvent.h
 * @brief Message published on the EventBus when something happens to a work item.
 *
 * Part of the **Observer** pattern (the event that observers receive).
 */
#ifndef WORKFLOW_EVENT_H
#define WORKFLOW_EVENT_H

#include <string>

/**
 * @class WorkFlowEvent
 * @brief Immutable record of "something happened" to a work item.
 *
 * The event type is a plain string (for example "STATE_CHANGED") rather than
 * an enum, so new event names can be added without changing this class.
 */
class WorkFlowEvent {
private:
    std::string eventName;   ///< Kind of event, e.g. "STATE_CHANGED".
    std::string instanceId;  ///< Workflow instance the event belongs to.
    std::string workItemId;  ///< Work item that caused the event.
    std::string payload;     ///< Human readable detail, e.g. "IDLE -> ASSIGNED".
    int sequence;            ///< Order in which events were created (starts at 1).

    static int nextSequence; ///< Counter used to hand out sequence numbers.

public:
    /**
     * @brief Creates an event and gives it the next sequence number.
     * @param eventName  Kind of event, e.g. "STATE_CHANGED".
     * @param instanceId Id of the workflow instance.
     * @param workItemId Id of the work item.
     * @param payload    Detail text describing what changed.
     */
    WorkFlowEvent(const std::string& eventName,
                  const std::string& instanceId,
                  const std::string& workItemId,
                  const std::string& payload);

    /**

     * @brief Returns the kind of event.

     * @return The kind of event.

     */
    std::string getEventName() const;
    /**
     * @brief Returns the id of the workflow instance.
     * @return The id of the workflow instance.
     */
    std::string getInstanceId() const;
    /**
     * @brief Returns the id of the work item.
     * @return The id of the work item.
     */
    std::string getWorkItemId() const;
    /**
     * @brief Returns the detail text.
     * @return The detail text.
     */
    std::string getPayload() const;
    /**
     * @brief Returns the sequence number (events are numbered in creation order).
     * @return The sequence number (events are numbered in creation order).
     */
    int getSequence() const;
};

#endif // WORKFLOW_EVENT_H
