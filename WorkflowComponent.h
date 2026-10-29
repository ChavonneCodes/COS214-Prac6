/**
 * @file WorkflowComponent.h
 * @brief Defines the WorkflowComponent abstract base class.
 * 
 * Part of the Composite pattern: this represents the base Component (node) 
 * in the static workflow definition tree.
 *
 * Also the Prototype in the Prototype pattern: every node can clone() itself
 * (deep copy), so an existing definition tree can be duplicated and edited
 * without rebuilding it from scratch.
 */

#ifndef WORKFLOW_COMPONeNT_H
#define WORKFLOW_COMPONeNT_H

#include <string>
#include <vector>
#include <iostream>

/**
 * @class WorkflowComponent
 * @brief The base class for all nodes in the workflow definition tree.
 * 
 * This abstract class defines the interface for both Leaf nodes (ActivityDefinition) 
 * and Composite nodes (StageDefinition, SubProcessDefinition). By treating both 
 * uniformly, the engine can navigate and execute the tree without needing to know 
 * the specific type of each node.
 */
class WorkflowComponent {
public:
    /**
     * @brief Virtual destructor to ensure proper cleanup of derived classes.
     */
    virtual ~WorkflowComponent() = default;

    /**
     * @brief Gets the unique identifier of the component.
     * @return The component ID.
     */
    virtual std::string getId() const = 0;

    /**
     * @brief Gets the human-readable name of the component.
     * @return The component name.
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Prints the details of the component to standard output.
     * @param indentLevel The indentation level for hierarchical display.
     */
    virtual void print(int indentLevel = 0) const = 0;

    // Composite Operations

    /**
     * @brief Adds a child component. Only valid for composite nodes.
     * @note By default, this does nothing and prints an error, as leaves cannot have children.
     * @param child Pointer to the child component to add (unused in the base class).
     */
    virtual void addChild(WorkflowComponent* /*child*/) {
        std::cout << "Cannot add child to a leaf component." << std::endl;
    }

    /**
     * @brief Retrieves the list of child components. Only valid for composite nodes.
     * @note By default, returns an empty vector, as leaves have no children.
     * @return A vector of pointers to the child components.
     */
    virtual std::vector<WorkflowComponent*> getChildren() const {
        return {};
    }

    // Prototype Operation

    /**
     * @brief Creates a deep copy of this component (Prototype pattern).
     *
     * The copy is completely independent of the original: for a composite
     * node every child is cloned recursively, so editing the copy (adding or
     * removing children) never changes the original tree, and vice versa.
     *
     * @note The caller owns the returned pointer and must delete it, or hand
     *       it to something that takes ownership (e.g. StageDefinition::addChild
     *       or WorkflowDefinition::setRoot).
     * @return A newly allocated copy of this component and its whole subtree.
     */
    virtual WorkflowComponent* clone() const = 0;
};

#endif // WORKFLOW_COMPONeNT_H