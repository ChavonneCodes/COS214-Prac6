/**
 * @file EventBus.cpp
 * @brief Implementation of EventBus (Observer pattern: Subject).
 */
#include "EventBus.h"
#include <algorithm>

void EventBus::attach(WorkFlowObserver* observer) {
    if (!observer) return;
    // Do not attach the same observer twice
    if (std::find(observers.begin(), observers.end(), observer) != observers.end()) return;
    observers.push_back(observer);
}

void EventBus::detach(WorkFlowObserver* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer),
                    observers.end());
}

void EventBus::notify(const WorkFlowEvent& event) {
    // Iterate over a copy so an observer may detach itself while being notified
    std::vector<WorkFlowObserver*> snapshot = observers;
    for (WorkFlowObserver* observer : snapshot) {
        observer->update(event);
    }
}

int EventBus::observerCount() const {
    return static_cast<int>(observers.size());
}
