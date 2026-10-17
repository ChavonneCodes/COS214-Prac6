#include "WorkItem.h"

WorkItem::WorkItem(const std::string& instanceId, ActivityDefinition* def)
    : instanceId(instanceId), definition(def), state("IDLE") {}

std::string WorkItem::getId() const { return definition->getId(); }
std::string WorkItem::getState() const { return state; }
void WorkItem::setState(const std::string& newState) { state = newState; }

void WorkItem::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "[WorkItem] " << definition->getName() 
              << " (State: " << state << ")\n";
}