#include "StageDefinition.h"

StageDefinition::StageDefinition(const std::string& id, const std::string& name)
    : stageId(id), name(name) {}

StageDefinition::~StageDefinition() {
    // Delete all children to avoid memory leaks
    for (WorkflowComponent* child : children) {
        delete child;
    }
    children.clear();
}

std::string StageDefinition::getId() const { return stageId; }
std::string StageDefinition::getName() const { return name; }

void StageDefinition::addChild(WorkflowComponent* child) {
    children.push_back(child);
}

std::vector<WorkflowComponent*> StageDefinition::getChildren() const {
    return children;
}

void StageDefinition::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "+ Stage: " << name << " (ID: " << stageId << ")\n";
    // Recursively print all children
    for (WorkflowComponent* child : children) {
        child->print(indentLevel + 1);
    }
}