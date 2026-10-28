/**
 * @file SubProcessInstance.h
 * @brief Defines the SubProcessInstance class.
 * 
 * Part of the Composite pattern: this represents a special Leaf node in the 
 * live Runtime tree that manages the execution of a nested workflow.
 */

#ifndef SUB_PROCESS_INSTANCE_H
#define SUB_PROCESS_INSTANCE_H

#include "RuntimeComponent.h"
#include "SubProcessDefinition.h"

class  WorkflowInstance;
class RuntimeVisitor;
/**
 * @class SubProcessInstance
 * @brief Represents a live, running instance of a sub-process call.
 * 
 * The SubProcessInstance is the runtime counterpart to the SubProcessDefinition. 
 * While it acts as a Leaf node within the parent runtime tree (it has no children 
 * of its own in that tree), it internally manages a completely separate nested 
 * WorkflowInstance.
 * 
 * When this node becomes active, it pauses the parent workflow, executes the 
 * called definition, and returns the output data to the parent once complete.
 */
class SubProcessInstance : public RuntimeComponent {
private:
    std::string instanceId;             ///< The ID of the parent workflow instance this sub-process belongs to.
    SubProcessDefinition* definition;   ///< Pointer to the static blueprint (not owned by this class).
    std::string state;                  ///< The current runtime state of the sub-process (e.g., "IDLE", "RUNNING").
    WorkflowInstance* childInstance=nullptr;

public:
    /**
     * @brief Constructs a new SubProcessInstance.
     * 
     * @param instanceId The unique ID of the overarching parent workflow instance.
     * @param def Pointer to the SubProcessDefinition blueprint this instance is based on.
     */
    SubProcessInstance(const std::string& instanceId, SubProcessDefinition* def);
    
    /**
     * @brief Retrieves the unique identifier of this sub-process node.
     * 
     * @return The sub-process ID (delegated from the definition).
     */
    std::string getId() const override;

    /**
     * @brief Retrieves the current runtime state of the sub-process.
     * 
     * @return The state as a string (e.g., "IDLE").
     */
    std::string getState() const override;

    /**
     * @brief Sets the current runtime state of the sub-process.
     * 
     * @param newState The new state string.
     */
    void setState(const std::string& newState) override;

    /**
     * @brief Prints the sub-process instance details to standard output.
     * 
     * Displays the ID of the called workflow definition and the current state.
     * 
     * @param indentLevel The number of indentation levels to apply (2 spaces per level).
     */
    void print(int indentLevel = 0) const override;

    /**
     * @brief Calls the visitor's visitSubProcess() for this node.
     *
     * @param visitor The operation to run (Visitor pattern).
     */
    void accept(RuntimeVisitor& visitor) override;

    /**
     * @brief Attaches the nested child instance to this node.
     * @note The WorkflowEngine is the sole owner of the child instance. 
     * This class holds a non-owning pointer for traversal and display purposes.
     * @param child Pointer to the child WorkflowInstance.
     */
    void setChildInstance(WorkflowInstance* child);

    /**
     * @brief Retrieves the nested child instance, if it has been started.
     * @return Pointer to the nested WorkflowInstance, or nullptr if not yet dispatched.
     */
    WorkflowInstance* getChildInstance() const;

    /**
     * @brief Retrieves the blueprint this sub-process is based on.
     * @return Pointer to the SubProcessDefinition.
     */
    SubProcessDefinition* getDefinition() const;

    
};

#endif // SUB_PROCESS_INSTANCE_H