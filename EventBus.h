/**
 * @file EventBus.h
 * @brief Central publisher that forwards workflow events to attached observers.
 *
 * Part of the **Observer** pattern (role: Subject).
 */
#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <vector>
#include "WorkFlowObserver.h"
#include "WorkFlowEvent.h"

/**
 * @class EventBus
 * @brief Subject of the Observer pattern.
 *
 * Work items publish events here and the bus forwards each one to every
 * attached observer. The bus does **not** own its observers (aggregation), so
 * the caller is responsible for their lifetime.
 */
class EventBus {
private:
    std::vector<WorkFlowObserver*> observers; ///< Attached observers (not owned).

public:
    /**
     * @brief Subscribes an observer.
     * @param observer Observer to add. Null pointers and observers that are
     *                 already attached are ignored.
     */
    void attach(WorkFlowObserver* observer);

    /**
     * @brief Unsubscribes an observer.
     * @param observer Observer to remove. Does nothing if it is not attached.
     */
    void detach(WorkFlowObserver* observer);

    /**
     * @brief Sends an event to every attached observer, in attach order.
     * @param event The event to publish.
     * @note Observers are notified from a copy of the list, so an observer may
     *       safely detach itself while being notified.
     */
    void notify(const WorkFlowEvent& event);

    /**

     * @brief Returns how many observers are currently attached.

     * @return How many observers are currently attached.

     */
    int observerCount() const;
};

#endif // EVENT_BUS_H
