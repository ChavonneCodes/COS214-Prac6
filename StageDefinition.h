/**
 * @file StageDefinition.h
 * @brief Defines the StageDefinition class.
 * 
 * Part of the Composite pattern: this represents a Composite (branch) node 
 * in the workflow definition tree.
 * Also a ConcretePrototype in the Prototype pattern (see clone()).
 */

#ifndef STAGE_DEFINITION_H
#define STAGE_DEFINITION_H

#include "WorkflowComponent.h"
#include <iostream>
#include <vector>

/**
 * @class StageDefinition
 * @brief Represents a grouping of work (a stage) within the static workflow blueprint.
 * 
 * A stage acts as a container for other WorkflowComponents. It can hold 
 * individual ActivityDefinition leaves, or even other nested StageDefinition 
 * objects, allowing for deeply nested workflow structures. 
 * 
 * As a Composite, it owns its children and is responsible for deleting them.
 */
class StageDefinition : public WorkflowComponent {
private:
    std::string stageId;                            ///< Unique identifier for the stage.
    std::string name;                               ///< Human-readable name of the stage.
    std::vector<WorkflowComponent*> children;       ///< List of child components (activities or nested stages). The stage owns these.

public:
    /**
     * @brief Constructs a new StageDefinition.
     * 
     * @param id The unique identifier for the stage.
     * @param name The human-readable name of the stage.
     */
    StageDefinition(const std::string& id, const std::string& name);

    /**
     * @brief Destructor that recursively deletes all child components.
     * 
     * Because the stage owns its children, it must clean them up to prevent memory leaks.
     */
    ~StageDefinition() override; 

    /**
     * @brief Retrieves the unique identifier of the stage.
     * 
     * @return The stage ID as a string.
     */
    std::string getId() const override;

    /**
     * @brief Retrieves the human-readable name of the stage.
     * 
     * @return The stage name as a string.
     */
    std::string getName() const override;

    /**
     * @brief Adds a child component to this stage.
     * 
     * The child can be an ActivityDefinition, another StageDefinition, or a 
     * SubProcessDefinition. The stage takes ownership of the pointer.
     * 
     * @param child Pointer to the child component to add.
     */
    void addChild(WorkflowComponent* child) override;

    /**
     * @brief Retrieves the list of child components.
     * 
     * @return A vector of pointers to the child components.
     */
    std::vector<WorkflowComponent*> getChildren() const override;

    /**
     * @brief Prints the stage and recursively prints all its children.
     * 
     * Useful for visually displaying the hierarchical tree structure in the console.
     * 
     * @param indentLevel The number of indentation levels to apply (2 spaces per level).
     */
    void print(int indentLevel = 0) const override;

    /**
     * @brief Creates a deep copy of this stage and everything below it (Prototype pattern).
     *
     * Because a stage owns its children through raw pointers, copying the
     * pointers would make two stages delete the same objects. Instead every
     * child is cloned (recursively, through its own clone()), so the copy owns
     * a completely separate subtree. Nested stages, activities and
     * sub-processes are all duplicated.
     *
     * @note The caller owns the returned pointer.
     * @return A new StageDefinition with the same id and name and cloned children.
     */
    StageDefinition* clone() const override;
};

#endif // STAGE_DEFINITION_H