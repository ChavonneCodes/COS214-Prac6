/**
 * @file LoggingRuntimeDecorator.h
 * @brief Runtime state-change logging.
 *
 * Part of the **Decorator** pattern (role: ConcreteDecorator).
 */
#ifndef LOGGING_RUNTIME_DECORATOR_H
#define LOGGING_RUNTIME_DECORATOR_H

#include "RuntimeComponentDecorator.h"
#include <iostream>

/**
 * @class LoggingRuntimeDecorator
 * @brief Logs state-change calls that return normally through this wrapper.
 *
 * Writes the component identifier and states before and after the call to
 * standard output. Changes made directly to the wrapped component are not logged.
 */
class LoggingRuntimeDecorator : public RuntimeComponentDecorator {
public:
    /**
     * @brief Creates a logging wrapper and takes ownership of its component.
     * @param component Heap-allocated component not owned elsewhere.
     * @throws std::invalid_argument If component is nullptr.
     */
    explicit LoggingRuntimeDecorator(RuntimeComponent* component)
        : RuntimeComponentDecorator(component) {}

    /**
     * @brief Delegates a state change, then logs the old and resulting states.
     * @param newState Requested state name.
     *
     * If the delegated call throws, the exception propagates and no log is written.
     * A normally returning call is logged even if the state remains unchanged.
     */
    void setState(const std::string& newState) override {
        const std::string oldState = getState();
        RuntimeComponentDecorator::setState(newState);
        std::cout << "[STATE] " << getId() << ": " << oldState
                  << " -> " << getState() << '\n';
    }
};

#endif
