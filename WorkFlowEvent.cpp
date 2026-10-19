/**
 * @file WorkFlowEvent.cpp
 * @brief Implementation of WorkFlowEvent (Observer pattern: the event message).
 */
#include "WorkFlowEvent.h"

int WorkFlowEvent::nextSequence = 1;

WorkFlowEvent::WorkFlowEvent(const std::string& eventName,
                             const std::string& instanceId,
                             const std::string& workItemId,
                             const std::string& payload)
    : eventName(eventName), instanceId(instanceId), workItemId(workItemId),
      payload(payload), sequence(nextSequence++) {}

std::string WorkFlowEvent::getEventName() const { return eventName; }
std::string WorkFlowEvent::getInstanceId() const { return instanceId; }
std::string WorkFlowEvent::getWorkItemId() const { return workItemId; }
std::string WorkFlowEvent::getPayload() const { return payload; }
int WorkFlowEvent::getSequence() const { return sequence; }
