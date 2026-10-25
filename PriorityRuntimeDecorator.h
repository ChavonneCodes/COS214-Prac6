/** @file PriorityRuntimeDecorator.h
 *  @brief ConcreteDecorator adding scheduling priority and ordered child access.
 */
#ifndef PRIORITY_RUNTIME_DECORATOR_H
#define PRIORITY_RUNTIME_DECORATOR_H

#include "RuntimeComponentDecorator.h"
#include <algorithm>

/**
 * Adds priority to any runtime node. On composites, getChildren() returns
 * immediate children in descending priority, preserving insertion order for
 * ties. The stored tree and print output are unchanged. Consumers must use
 * getChildren() to obtain this ordering; this does not execute work itself.
 */
class PriorityRuntimeDecorator : public RuntimeComponentDecorator {
private:
    int priority;

public:
    /** @brief Takes ownership of component; any integer priority is valid. */
    PriorityRuntimeDecorator(RuntimeComponent* component, int priority)
        : RuntimeComponentDecorator(component), priority(priority) {}

    int getPriority() const override { return priority; }

    /** @brief Changes priority for subsequent child-order queries by its parent. */
    void setPriority(int value) { priority = value; }

    std::vector<RuntimeComponent*> getChildren() const override {
        std::vector<RuntimeComponent*> children = RuntimeComponentDecorator::getChildren();

        for (int i = 0; i < children.size(); i++) {
            for (int j = i + 1; j < children.size(); j++) {
                RuntimeComponent* a = children.at(i);
                RuntimeComponent* b = children.at(j);

                if (a == nullptr && b != nullptr) {
                    std::swap(children.at(i), children.at(j));
                }
                else if (a != nullptr && b != nullptr) {
                    if (a->getPriority() < b->getPriority()) {
                        std::swap(children.at(i), children.at(j));
                    }
                }
            }
        }

        return children;
    }
};

#endif
