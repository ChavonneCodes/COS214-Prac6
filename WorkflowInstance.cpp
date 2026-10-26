#include "WorkflowInstance.h"

namespace {

// Walks the Composite tree and collects its leaves.
// TODO: replace with the RunTimeIterator once the Iterator pattern is in.
void collectWorkItems(RuntimeComponent* node, std::vector<WorkItem*>& out) {
    if (!node) return;
    WorkItem* item = dynamic_cast<WorkItem*>(node);
    if (item) out.push_back(item);
    for (RuntimeComponent* child : node->getChildren()) {
        collectWorkItems(child, out);
    }
}

} // namespace

WorkflowInstance::WorkflowInstance(const std::string& instanceId, const std::string& definitionId,
                                   int definitionVersion, RuntimeComponent* root, EventBus* eventBus)
    : instanceId(instanceId), definitionId(definitionId), definitionVersion(definitionVersion),
      root(root), eventBus(eventBus) {}

WorkflowInstance::~WorkflowInstance() {
    delete root;
}

std::string WorkflowInstance::getInstanceId() const { return instanceId; }
std::string WorkflowInstance::getDefinitionId() const { return definitionId; }
int WorkflowInstance::getDefinitionVersion() const { return definitionVersion; }
RuntimeComponent* WorkflowInstance::getRoot() const { return root; }

std::vector<WorkItem*> WorkflowInstance::getWorkItems() const {
    std::vector<WorkItem*> items;
    collectWorkItems(root, items);
    return items;
}

WorkItem* WorkflowInstance::findWorkItem(const std::string& activityId) const {
    for (WorkItem* item : getWorkItems()) {
        if (item->getId() == activityId) return item;
    }
    return nullptr;
}

std::string WorkflowInstance::getState() const { return state; }

void WorkflowInstance::setState(const std::string& newState) {
    std::string oldState = state;
    state = newState;
    if (eventBus) {
        eventBus->notify(WorkFlowEvent("INSTANCE_STATE_CHANGED", instanceId, "-",
                                       oldState + " -> " + newState));
    }
}

Data& WorkflowInstance::getData() { return data; }
