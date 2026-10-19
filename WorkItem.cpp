#include "WorkItem.h"

WorkItem::WorkItem(const std::string& instanceId, ActivityDefinition* def)
    : instanceId(instanceId), definition(def), state("IDLE") {}

// ---- Strategy support ----

std::string WorkItem::getRequiredRoleId() const { return definition->getRequiredRoleId(); }

Participant* WorkItem::getAssignedParticipant() const { return assignedParticipant; }

bool WorkItem::assignTo(Participant* participant) {
    if (!participant) return false;
    if (assignedParticipant) assignedParticipant->closeOpenItem();  // reassignment

    assignedParticipant = participant;
    assignedParticipant->addOpenItem();
    setState("ASSIGNED");
    return true;
}

std::string WorkItem::getId() const { return definition->getId(); }
std::string WorkItem::getState() const { return state; }
void WorkItem::setState(const std::string& newState) { state = newState; }

void WorkItem::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "[WorkItem] " << definition->getName() 
              << " (State: " << state << ")";
    if (assignedParticipant) {
        std::cout << " (Assigned: " << assignedParticipant->getName() << ")";
    }
    std::cout << "\n";
}