/**
 * @file LabelledRuntimeDecorator.h
 * @brief Display labels for runtime components and composite subtrees.
 *
 * Part of the **Decorator** pattern (role: ConcreteDecorator).
 */
#ifndef LABELLED_RUNTIME_DECORATOR_H
#define LABELLED_RUNTIME_DECORATOR_H

#include "RuntimeComponentDecorator.h"
#include <iostream>

/**
 * @class LabelledRuntimeDecorator
 * @brief Prints a label above a leaf or an entire composite subtree.
 */
class LabelledRuntimeDecorator : public RuntimeComponentDecorator {
private:
    std::string label; ///< Display text printed in square brackets.

public:
    /**
     * @brief Creates a labelled wrapper and takes ownership of its component.
     * @param component Heap-allocated component not owned elsewhere.
     * @param label Display text to copy into the wrapper.
     * @throws std::invalid_argument If component is nullptr.
     */
    LabelledRuntimeDecorator(RuntimeComponent* component,
                             const std::string& label)
        : RuntimeComponentDecorator(component), label(label) {}

    /**
     * @brief Prints the label, then the component one indentation level deeper.
     * @param indentLevel Non-negative label indentation level, using two spaces per level.
     */
    void print(int indentLevel = 0) const override {
        std::cout << std::string(indentLevel * 2, ' ')
                  << "[" << label << "]\n";
        RuntimeComponentDecorator::print(indentLevel + 1);
    }
};

#endif
