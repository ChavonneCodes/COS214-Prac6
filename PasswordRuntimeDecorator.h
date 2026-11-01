#ifndef PASSWORD_RUNTIME_DECORATOR_H
#define PASSWORD_RUNTIME_DECORATOR_H

#include "RuntimeComponentDecorator.h"
#include "Participant.h"

/** In-memory demo access key, separate from participant roles.
 * Unlocks only for the current actor; changing actors or entering a wrong key
 * locks it again. Reads never expose the key. This is not credential storage
 * or authentication. As with other wrappers, retained raw pointers bypass it.
 */
class PasswordRuntimeDecorator : public RuntimeComponentDecorator {
    std::string expectedKey;
    std::string keyName;
    const Participant* actor = nullptr; // non-owning
    bool unlocked = false;
public:
    PasswordRuntimeDecorator(RuntimeComponent* component, const std::string& key,
                             const std::string& name = "")
        : RuntimeComponentDecorator(component), expectedKey(key), keyName(name) {
        if (key.empty()) throw std::invalid_argument("Access key must not be empty");
    }

    void setActor(const Participant* participant) override {
        if (actor != participant) lock();
        actor = participant;
        wrapped->setActor(participant);
    }
    bool enterKey(const std::string& key) {
        unlocked = actor && key == expectedKey;
        return unlocked;
    }
    void lock() { unlocked = false; }
    bool canModify() const { return actor && unlocked; }
    std::string getName() const override {
        return wrapped->getName() + " [requires access key" +
            (keyName.empty() ? "" : ": " + keyName) + "]";
    }
    void checkAccess() const override {
        if (!canModify()) {
            const std::string who = actor ? actor->getName() : "no participant selected";
            throw std::runtime_error("missing access key for " + who + ": " +
                (keyName.empty() ? "component access key" : keyName) + "; use Enter access key");
        }
        wrapped->checkAccess();
    }
};

#endif
