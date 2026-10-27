/**
 * @file RuntimeComponentDecorator.h
 * @brief Owning wrapper that forwards runtime component operations.
 *
 * Part of the **Decorator** pattern (role: Decorator).
 */
#ifndef RUNTIME_COMPONENT_DECORATOR_H
#define RUNTIME_COMPONENT_DECORATOR_H

#include "RuntimeComponent.h"
#include <memory>
#include <stdexcept>

/**
 * @class RuntimeComponentDecorator
 * @brief Base decorator for runtime leaves, composites, and other decorators.
 *
 * Preserves the RuntimeComponent interface and delegates operations to the
 * wrapped component. Destroying this decorator also destroys its component.
 */
class RuntimeComponentDecorator : public RuntimeComponent {
protected:
    std::unique_ptr<RuntimeComponent> wrapped; ///< Exclusively owned component.

public:
    /**
     * @brief Takes exclusive ownership of a runtime component.
     * @param component Heap-allocated component to wrap; must not be owned elsewhere.
     * @throws std::invalid_argument If component is nullptr.
     */
    explicit RuntimeComponentDecorator(RuntimeComponent* component)
        : wrapped(component) {
        if (!wrapped) {
            throw std::invalid_argument("Cannot decorate nullptr");
        }
    }

    /**
     * @brief Returns the wrapped component's identifier.
     * @return The wrapped component's identifier.
     */
    std::string getId() const override {
        return wrapped->getId();
    }

    /**
     * @brief Returns the wrapped component's current state.
     * @return The state reported by the wrapped component.
     */
    std::string getState() const override {
        return wrapped->getState();
    }

    /** @brief Preserve priority when another decorator is stacked outside it. */
    int getPriority() const override {
        return wrapped->getPriority();
    }

    /**
     * @brief Forwards a state change to the wrapped component.
     * @param newState Requested state name.
     */
    void setState(const std::string& newState) override {
        wrapped->setState(newState);
    }

    /**
     * @brief Prints the wrapped component at the requested indentation level.
     * @param indentLevel Indentation level passed to the wrapped component.
     */
    void print(int indentLevel = 0) const override {
        wrapped->print(indentLevel);
    }

    /**
     * @brief Delegates child insertion to the wrapped component.
     * @param child Child to add; ownership follows the wrapped component's contract.
     *
     * Wrapping a leaf does not give it support for children.
     */
    void addChild(RuntimeComponent* child) override {
        wrapped->addChild(child);
    }

    /**
     * @brief Returns the children exposed by the wrapped component.
     * @return Non-owning child pointers, or an empty vector for a leaf.
     */
    std::vector<RuntimeComponent*> getChildren() const override {
        return wrapped->getChildren();
    }

    /**
     * @brief Passes a visitor to the wrapped component.
     * @param visitor The operation to run (Visitor pattern).
     *
     * The decorator itself is invisible to visitors: they see the wrapped node.
     */
    void accept(RuntimeVisitor& visitor) override {
        wrapped->accept(visitor);
    }
};

#endif
