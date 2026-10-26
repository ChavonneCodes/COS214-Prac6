#include "InMemoryDefinitionRepository.h"

InMemoryDefinitionRepository::~InMemoryDefinitionRepository() {
    for (WorkflowDefinition* d : definitions) delete d;
}

void InMemoryDefinitionRepository::save(WorkflowDefinition* definition) {
    // TODO
    definitions.push_back(definition);
}

WorkflowDefinition* InMemoryDefinitionRepository::load(const std::string& defId, int version) const {
    for (WorkflowDefinition* d : definitions) {
        if (d->getDefinitionId() == defId && d->getVersion() == version) {
            return d;
        }
    }
    return nullptr;
}

WorkflowDefinition* InMemoryDefinitionRepository::loadLatest(const std::string& defId) const {
    WorkflowDefinition* latest = nullptr;
    for (WorkflowDefinition* d : definitions) {
        if (d->getDefinitionId() == defId &&
            (latest == nullptr || d->getVersion() > latest->getVersion())) {
            latest = d;
        }
    }
    return latest;
}

const std::vector<WorkflowDefinition*>& InMemoryDefinitionRepository::findAll() const {
    return definitions;
}
