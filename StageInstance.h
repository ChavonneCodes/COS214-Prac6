/**
 * @file StageInstance.h
 * @brief Defines the StageInstance class.
 * 
 * Part of the Composite pattern: this represents a Composite (branch) node 
 * in the live Runtime tree.
 */

#ifndef STAGE_INSTANCE_H
#define STAGE_INSTANCE_H

#include "RuntimeComponent.h"
#include "StageDefinition.h"
#include <vector>

/**
 * @class StageInstance
 * @brief Represents a live, running instance of a workflow stage.
 * 
 * The StageInstance is the runtime counterpart to the static StageDefinition. 
 * It acts as a container for other runtime components (WorkItems or nested 
 * StageInstances). 
 * 
 * It holds a reference to its blueprint (the StageDefinition) to know what it 
 * is supposed to do, but it maintains its own mutable state and dynamically 
 * owns its child runtime components.
 */
class StageInstance : public RuntimeComponent {
private:
    std::string instanceId;                         ///< The ID of the parent workflow instance this stage belongs to.
    StageDefinition* definition;                    ///< Pointer to the static blueprint (not owned by this class).
    std::string state;                              ///< The current runtime state of the stage (e.g., "IDLE", "RUNNING").
    std::vector<RuntimeComponent*> children;        ///< The list of active child components. The instance owns these.

public:
    /**
     * @brief Constructs a new StageInstance.
     * 
     * @param instanceId The unique ID of the overarching workflow instance.
     * @param def Pointer to the StageDefinition blueprint this instance is based on.
     */
    StageInstance(const std::string& instanceId, StageDefinition* def);

    /**
     * @brief Destructor that recursively deletes all child components.
     * 
     * Because the StageInstance owns its runtime children, it must clean them up 
     * to prevent memory leaks when the workflow finishes or is cancelled.
     */
    ~StageInstance() override; 

    /**
     * @brief Retrieves the unique identifier of the stage.
     * 
     * @return The stage ID (delegated from the definition).
     */
    std::string getId() const override;

    /**
     * @brief Retrieves the current runtime state of the stage.
     * 
     * @return The state as a string (e.g., "IDLE").
     */
    std::string getState() const override;

    /**
     * @brief Sets the current runtime state of the stage.
     * 
     * @param newState The new state string.
     */
    void setState(const std::string& newState) override;

    /**
     * @brief Prints the stage instance and recursively prints all its children.
     * 
     * @param indentLevel The number of indentation levels to apply (2 spaces per level).
     */
    void print(int indentLevel = 0) const override;

    /**
     * @brief Adds a runtime child component (WorkItem or nested StageInstance) to this stage.
     * 
     * The StageInstance takes ownership of the pointer.
     * 
     * @param child Pointer to the runtime component to add.
     */
    void addChild(RuntimeComponent* child) override;

    /**
     * @brief Retrieves the list of active child components.
     * 
     * @return A vector of pointers to the child runtime components.
     */
    std::vector<RuntimeComponent*> getChildren() const override;
};

#endif // STAGE_INSTANCE_H