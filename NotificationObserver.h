/**
 * @file NotificationObserver.h
 * @brief Observer that sends a notification for one chosen kind of event.
 *
 * Part of the **Observer** pattern (role: ConcreteObserver).
 */
#ifndef NOTIFICATION_OBSERVER_H
#define NOTIFICATION_OBSERVER_H

#include <string>
#include <vector>
#include "WorkFlowObserver.h"

/**
 * @class NotificationObserver
 * @brief Reacts only to the event name it watches and "notifies" someone.
 *
 * Messages are printed and also stored so the behaviour can be tested.
 */
class NotificationObserver : public WorkFlowObserver {
private:
    std::string watchedEvent;               ///< Only events with this name trigger a message.
    std::vector<std::string> notifications; ///< Messages produced so far.

public:
    /**
     * @brief Creates an observer that watches one kind of event.
     * @param watchedEvent Event name to react to, e.g. "STATE_CHANGED".
     */
    explicit NotificationObserver(const std::string& watchedEvent);

    /**
     * @brief Produces a notification if the event is the one being watched.
     * @param event The event that was published. Other event names are ignored.
     */
    void update(const WorkFlowEvent& event) override;

    /**

     * @brief Returns a copy of every notification produced so far.

     * @return A copy of every notification produced so far.

     */
    std::vector<std::string> getNotifications() const;
};

#endif // NOTIFICATION_OBSERVER_H
