/**
 * @file SubProcessDefinition.h
 * @brief Defines the SubProcessDefinition class.
 * 
 * Part of the Composite pattern: this represents a special Leaf node in the 
 * workflow definition tree that calls another workflow definition.
 */

#ifndef SUB_PROCESS_DEFINITION_H
#define SUB_PROCESS_DEFINITION_H

#include "WorkflowComponent.h"
#include <iostream>

/**
 * @class SubProcessDefinition
 * @brief Represents a call to another workflow definition within the current blueprint.
 * 
 * Unlike a StageDefinition which contains its children inline, a SubProcessDefinition 
 * acts as a portal or reference. It does not contain internal nodes; instead, it 
 * points to a completely separate WorkflowDefinition by ID and version. 
 * 
 * When the engine reaches this node during execution, it pauses the current 
 * workflow and runs the referenced definition as a nested instance.
 */
class SubProcessDefinition : public WorkflowComponent {
private:
    std::string subProcessId;       ///< Unique identifier for this node within the parent workflow.
    std::string calledDefinitionId; ///< The ID of the external workflow definition being invoked.
    int calledVersion;              ///< The specific version of the external workflow to run.

public:
    /**
     * @brief Constructs a new SubProcessDefinition.
     * 
     * @param id The unique identifier for this sub-process node.
     * @param calledDefId The ID of the workflow definition to call.
     * @param version The version of the workflow definition to call.
     */
    SubProcessDefinition(const std::string& id, const std::string& calledDefId, int version);
    
    /**
     * @brief Retrieves the unique identifier of this sub-process node.
     * 
     * @return The sub-process node ID as a string.
     */
    std::string getId() const override;

    /**
     * @brief Retrieves a formatted name describing the sub-process call.
     * 
     * @return A string formatted as "Call SubProcess: [calledDefinitionId]".
     */
    std::string getName() const override;

    /**
     * @brief Retrieves the ID of the external workflow definition being called.
     * 
     * Used by the engine to load the correct blueprint from the repository.
     * 
     * @return The called definition ID as a string.
     */
    std::string getCalledDefinitionId() const;

    /**
     * @brief Retrieves the version of the external workflow definition being called.
     * 
     * @return The called definition version as an integer.
     */
    int getCalledVersion() const;

    /**
     * @brief Prints the details of the sub-process call to standard output.
     * 
     * @param indentLevel The number of indentation levels to apply (2 spaces per level).
     */
    void print(int indentLevel = 0) const override;
};

#endif // SUB_PROCESS_DEFINITION_H