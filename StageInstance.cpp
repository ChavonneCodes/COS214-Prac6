#include "StageInstance.h"

StageInstance::StageInstance(const std::string& instanceId, StageDefinition* def)
    : instanceId(instanceId), definition(def), state("IDLE") {}

StageInstance::~StageInstance() {
    for (RuntimeComponent* child : children) {
        delete child;
    }
    children.clear();
}

std::string StageInstance::getId() const { return definition->getId(); }
std::string StageInstance::getState() const { return state; }
void StageInstance::setState(const std::string& newState) { state = newState; }

void StageInstance::addChild(RuntimeComponent* child) {
    children.push_back(child);
}

std::vector<RuntimeComponent*> StageInstance::getChildren() const {
    return children;
}

void StageInstance::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "[Stage] " << definition->getName() 
              << " (State: " << state << ")\n";
    for (RuntimeComponent* child : children) {
        child->print(indentLevel + 1);
    }
}