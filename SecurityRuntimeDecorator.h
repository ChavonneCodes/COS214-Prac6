/** @file SecurityRuntimeDecorator.h
 *  @brief ConcreteDecorator authorizing runtime mutations by participant role.
 */
#ifndef SECURITY_RUNTIME_DECORATOR_H
#define SECURITY_RUNTIME_DECORATOR_H

#include "RuntimeComponentDecorator.h"
#include "Participant.h"

/**
 * Requires an acting participant with the configured role for setState() and
 * addChild(). Reads remain public. The trusted caller supplies the actor; this
 * is authorization, not authentication. The participant is not owned and must
 * outlive its use here. Checks apply only to calls through this wrapper, not
 * retained raw pointers, WorkItem commands, or children returned by getChildren().
 * Decorate children separately when they also need mutation checks.
 */
class SecurityRuntimeDecorator : public RuntimeComponentDecorator {
private:
    std::string requiredRoleId;
    const Participant* actor;

    void requireAccess() const {
        if (!actor) {
            throw std::runtime_error("no participant assigned or selected; required role: " + requiredRoleId);
        }
        if (!actor->hasRole(requiredRoleId)) {
            std::string roles;
            for (const auto& role : actor->getRoleIds()) {
                if (!roles.empty()) roles += ", ";
                roles += role;
            }
            if (roles.empty()) roles = "none";
            throw std::runtime_error("unauthorised participant " + actor->getName() +
                " (roles: " + roles + "); required role: " + requiredRoleId);
        }
    }

public:
    /** @brief Takes ownership of component; null actor denies all mutations. */
    SecurityRuntimeDecorator(RuntimeComponent* component,
                             const std::string& requiredRoleId,
                             const Participant* actor = nullptr)
        : RuntimeComponentDecorator(component), requiredRoleId(requiredRoleId), actor(actor) {
        if (requiredRoleId.empty()) {
            throw std::invalid_argument("Security role must not be empty");
        }
        wrapped->setActor(actor);
    }

    /** @brief Sets the acting participant from the trusted calling context. */
    void setActor(const Participant* participant) override {
        actor = participant;
        wrapped->setActor(participant);
    }

    bool canModify() const {
        return actor && actor->hasRole(requiredRoleId);
    }

    std::string getName() const override {
        return wrapped->getName() + " [requires role: " + requiredRoleId + "]";
    }

    void checkAccess() const override {
        requireAccess();
        wrapped->checkAccess();
    }

    /** @throws std::runtime_error Before changing state if access is denied. */
    void setState(const std::string& newState) override {
        requireAccess();
        RuntimeComponentDecorator::setState(newState);
    }

    /**
     * @throws std::runtime_error If denied; caller retains ownership of child.
     * On success, ownership follows the wrapped component's contract.
     */
    void addChild(RuntimeComponent* child) override {
        requireAccess();
        RuntimeComponentDecorator::addChild(child);
    }
};

#endif
