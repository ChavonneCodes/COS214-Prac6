#include "WorkItem.h"

WorkItem::WorkItem(const std::string& instanceId, ActivityDefinition* def)
    : instanceId(instanceId), definition(def), state("IDLE") {}

std::string WorkItem::getId() const { return definition->getId(); }
std::string WorkItem::getState() const { return state; }
void WorkItem::setState(const std::string& newState) {
    std::string oldState = state;
    state = newState;
    publish("STATE_CHANGED", oldState + " -> " + newState);
}

// ---- Observer support ----

void WorkItem::setEventBus(EventBus* bus) { eventBus = bus; }

std::string WorkItem::getInstanceId() const { return instanceId; }

void WorkItem::publish(const std::string& eventName, const std::string& payload) {
    if (!eventBus) return;  // no bus attached: behave exactly as before
    eventBus->notify(WorkFlowEvent(eventName, instanceId, getId(), payload));
}

void WorkItem::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "[WorkItem] " << definition->getName() 
              << " (State: " << state << ")\n";
}