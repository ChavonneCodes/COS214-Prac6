/**
 * @file NotificationObserver.cpp
 * @brief Implementation of NotificationObserver (Observer pattern: ConcreteObserver).
 */
#include "NotificationObserver.h"
#include <iostream>

NotificationObserver::NotificationObserver(const std::string& watchedEvent)
    : watchedEvent(watchedEvent) {}

void NotificationObserver::update(const WorkFlowEvent& event) {
    if (event.getEventName() != watchedEvent) return;

    std::string message = "NOTIFY: '" + event.getWorkItemId() + "' -> " + event.getPayload();
    notifications.push_back(message);
    std::cout << message << "\n";
}

std::vector<std::string> NotificationObserver::getNotifications() const {
    return notifications;
}
