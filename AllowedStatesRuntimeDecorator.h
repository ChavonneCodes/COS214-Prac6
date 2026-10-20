/**
 * @file AllowedStatesRuntimeDecorator.h
 * @brief Validation of requested runtime state names.
 *
 * Part of the **Decorator** pattern (role: ConcreteDecorator).
 */
#ifndef ALLOWED_STATES_RUNTIME_DECORATOR_H
#define ALLOWED_STATES_RUNTIME_DECORATOR_H

#include "RuntimeComponentDecorator.h"
#include <set>

/**
 * @class AllowedStatesRuntimeDecorator
 * @brief Restricts state names requested through this wrapper.
 *
 * Does not enforce transition order or validate the component's initial state.
 * Direct calls to the wrapped component bypass this validation.
 */
class AllowedStatesRuntimeDecorator : public RuntimeComponentDecorator {
private:
    std::set<std::string> allowedStates; ///< Accepted state names (case-sensitive).

public:
    /**
     * @brief Creates a validating wrapper and takes ownership of its component.
     * @param component Heap-allocated component not owned elsewhere.
     * @param states Allowed names to copy; an empty set rejects all state changes.
     * @throws std::invalid_argument If component is nullptr.
     */
    AllowedStatesRuntimeDecorator(RuntimeComponent* component,
                                  const std::set<std::string>& states)
        : RuntimeComponentDecorator(component), allowedStates(states) {}

    /**
     * @brief Validates the requested name before forwarding the state change.
     * @param newState Requested state name, compared exactly against the allowed set.
     * @throws std::invalid_argument If newState is not allowed; the component is unchanged.
     *
     * Accepted names remain subject to the wrapped component's own behavior.
     */
    void setState(const std::string& newState) override {
        if (allowedStates.count(newState) == 0) {
            throw std::invalid_argument("State not allowed: " + newState);
        }
        RuntimeComponentDecorator::setState(newState);
    }
};

#endif
