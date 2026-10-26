#include "WorkflowInstance.h"
#include <iostream>

WorkflowInstance::WorkflowInstance(const std::string& id, const std::string& defId, int version, 
                                   RuntimeComponent* rootTree, EventBus* bus)
    : instanceId(id), definitionId(defId), definitionVersion(version), 
      root(rootTree), eventBus(bus), state("CREATED") {}

WorkflowInstance::~WorkflowInstance() {
    if (root) delete root;
}

std::string WorkflowInstance::getId() const { return instanceId; }
std::string WorkflowInstance::getDefinitionId() const { return definitionId; }
int WorkflowInstance::getDefinitionVersion() const { return definitionVersion; }
std::string WorkflowInstance::getState() const { return state; }
void WorkflowInstance::setState(const std::string& newState) { state = newState; }
RuntimeComponent* WorkflowInstance::getRoot() const { return root; }
Data& WorkflowInstance::getData() { return data; }

void collectWorkItems(RuntimeComponent* node, std::vector<WorkItem*>& items) {
    if (!node) return;
    WorkItem* item = dynamic_cast<WorkItem*>(node);
    if (item) items.push_back(item);
    for (RuntimeComponent* child : node->getChildren()) {
        collectWorkItems(child, items);
    }
}

std::vector<WorkItem*> WorkflowInstance::getWorkItems() const {
    std::vector<WorkItem*> items;
    collectWorkItems(root, items);
    return items;
}

WorkItem* WorkflowInstance::findWorkItem(const std::string& workItemId) const {
    for (WorkItem* item : getWorkItems()) {
        if (item->getId() == workItemId) {
            return item;
        }
    }
    return nullptr;
}


void WorkflowInstance::print() const {
    std::cout << "Instance: " << instanceId << " (" << definitionId << " v" << definitionVersion << ")\n";
    std::cout << "State: " << state << "\n";
    if (root) {
        root->print(0); // Delegates to the Composite tree
    }
}