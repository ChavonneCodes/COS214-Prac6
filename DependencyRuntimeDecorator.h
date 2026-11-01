#ifndef DEPENDENCY_RUNTIME_DECORATOR_H
#define DEPENDENCY_RUNTIME_DECORATOR_H

#include "RuntimeComponentDecorator.h"

/** Live prerequisites for mutations and commands. The callback returns the
 * reason access is blocked, or an empty string when prerequisites are met.
 * Captured dependencies must outlive this wrapper. Reads remain public.
 */
class DependencyRuntimeDecorator : public RuntimeComponentDecorator {
    std::string description;
    std::function<std::string()> blocker;
public:
    DependencyRuntimeDecorator(RuntimeComponent* component, const std::string& description,
                               const std::function<std::string()>& blocker)
        : RuntimeComponentDecorator(component), description(description), blocker(blocker) {
        if (!blocker) throw std::invalid_argument("Dependency check must not be empty");
    }
    std::string getName() const override {
        return wrapped->getName() + " [dependencies: " + description + "]";
    }
    std::string getBlockReason() const { return blocker(); }
    void checkAccess() const override {
        const std::string reason = blocker();
        if (!reason.empty()) throw std::runtime_error(reason);
        wrapped->checkAccess();
    }
};

#endif
