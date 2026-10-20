#include "InMemoryDefinitionRepository.h"

InMemoryDefinitionRepository::~InMemoryDefinitionRepository() {
    for (WorkflowDefinition* d : definitions) delete d;
}

void InMemoryDefinitionRepository::save(WorkflowDefinition* definition) {
    // TODO
    definitions.push_back(definition);
}

WorkflowDefinition* InMemoryDefinitionRepository::load(const std::string& defId, int version) const {
    // TODO
    (void)defId;
    (void)version;
    return nullptr;
}

WorkflowDefinition* InMemoryDefinitionRepository::loadLatest(const std::string& defId) const {
    // TODO
    (void)defId;
    return nullptr;
}

const std::vector<WorkflowDefinition*>& InMemoryDefinitionRepository::findAll() const {
    return definitions;
}
