/**
 * @file RuntimeComponent.h
 * @brief Defines the RuntimeComponent abstract base class.
 */

#ifndef RUNTIME_COMPONENT_H
#define RUNTIME_COMPONENT_H

#include "Iterator.h"
#include <string>
#include <vector>

class RuntimeVisitor;

/**
 * @class RuntimeComponent
 * @brief The base Component in the Composite pattern for running workflow instances.
 *
 * This class defines the interface for all nodes in the runtime tree (both leaves
 * like WorkItem and composites like StageInstance). It allows the engine to treat
 * individual tasks and groups of tasks uniformly.
 */
class RuntimeComponent : public Iterable<RuntimeComponent> {
public:
    /**
     * @brief Virtual destructor to ensure proper cleanup of derived classes.
     */
    virtual ~RuntimeComponent() = default;

    /**
     * @brief Gets the unique identifier of the component.
     * @return The component ID.
     */
    virtual std::string getId() const = 0;

    /**
     * @brief Gets the current execution state of the component.
     * @return The state as a string (e.g., "IDLE", "RUNNING", "SUCCESS").
     */
    virtual std::string getState() const = 0;

    /** @brief Scheduling priority; larger values come first. Undecorated nodes use 0. */
    virtual int getPriority() const { return 0; }

    /**
     * @brief Sets the current execution state of the component.
     * @param newState The new state string.
     */
    virtual void setState(const std::string& newState) = 0;

    /**
     * @brief Prints the details of the component to standard output.
     * @param indentLevel The indentation level for formatting hierarchical output.
     */
    virtual void print(int indentLevel = 0) const = 0;

    /**
     * @brief Accepts a visitor (Visitor pattern, role: Element).
     *
     * Each node calls the visit method for its own class. Composite nodes also
     * pass the visitor on to their children.
     * @param visitor The operation to run on this node.
     */
    virtual void accept(RuntimeVisitor& visitor) = 0;

    // --- Composite Operations ---

    /**
     * @brief Adds a child component. Only valid for composite nodes.
     * @note By default, this does nothing and prints an error, as leaves cannot have children.
     * @param child Pointer to the child component to add (unused in the base class).
     */
    virtual void addChild(RuntimeComponent* /*child*/);

    /**
     * @brief Retrieves the list of child components. Only valid for composite nodes.
     * @note By default, returns an empty vector, as leaves have no children.
     * @return A vector of pointers to the child components.
     */
    virtual std::vector<RuntimeComponent*> getChildren() const;

    // Composite operations
    virtual void addChild(RuntimeComponent* ) {
        std::cout << "Cannot add child to a leaf runtime component." << std::endl;
    }
    virtual std::vector<RuntimeComponent*> getChildren() const {
        return {};
    }

    RuntimeComponentIterator* createIterator(TraversalStrategy<RuntimeComponent>* strat) const override
    {
	    return new RuntimeComponentIterator(*this, strat);
    }
};

#endif //RUNTIME_COMPONENT_H
