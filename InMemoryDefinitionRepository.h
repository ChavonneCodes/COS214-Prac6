#ifndef INMEMORYDEFINITIONREPOSITORY_H
#define INMEMORYDEFINITIONREPOSITORY_H

#include <string>
#include <vector>
#include "WorkflowDefinition.h"

/**
 * @brief Stores every version of every WorkflowDefinition in memory.
 *
 * Used by the WorkflowEngine (Facade) to look up definitions. Keeping every
 * version lets a running instance stay pinned to the version it started with,
 * while new instances use the latest version.
 *
 * The repository owns the definitions it stores. It works with pointers
 * because WorkflowDefinition cannot be copied.
 */
class InMemoryDefinitionRepository {
private:
    std::vector<WorkflowDefinition*> definitions; ///< Every stored version (owned).

public:
    /**
     * @brief Deletes every stored definition.
     */
    ~InMemoryDefinitionRepository();

    /**
     * @brief Stores a definition. The repository takes ownership of it.
     * @param definition Definition to store.
     */
    void save(WorkflowDefinition* definition);

    /**
     * @brief Finds one specific version of a definition.
     * @param defId   Id of the definition.
     * @param version Version number to load.
     * @return The definition, or nullptr if it is not stored.
     */
    WorkflowDefinition* load(const std::string& defId, int version) const;

    /**
     * @brief Finds the highest version of a definition.
     * @param defId Id of the definition.
     * @return The latest version, or nullptr if none is stored.
     */
    WorkflowDefinition* loadLatest(const std::string& defId) const;

    /**
     * @brief Returns every stored definition, all versions included.
     * @return The stored definitions (still owned by the repository).
     */
    const std::vector<WorkflowDefinition*>& findAll() const;

    InMemoryDefinitionRepository() = default;
    InMemoryDefinitionRepository(const InMemoryDefinitionRepository&) = delete;
    InMemoryDefinitionRepository& operator=(const InMemoryDefinitionRepository&) = delete;
};

#endif
