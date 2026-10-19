/**
 * @file test_observer.cpp
 * @brief Functionality test for the Observer pattern (EventBus, observers, WorkItem events).
 */
// Functionality test for the OBSERVER pattern (EventBus + observers + WorkItem).
// Build from the repo folder (one line):
// g++ -std=c++11 -o test_observer test_observer.cpp WorkItem.cpp ActivityDefinition.cpp EventBus.cpp WorkFlowEvent.cpp AuditObserver.cpp NotificationObserver.cpp
#include <iostream>
#include <vector>

#include "WorkItem.h"
#include "ActivityDefinition.h"
#include "EventBus.h"
#include "AuditObserver.h"
#include "NotificationObserver.h"

static int passed = 0;
static int failed = 0;

#define CHECK(cond, msg)                                                  \
    do {                                                                  \
        if (cond) { ++passed; std::cout << "  [PASS] " << msg << "\n"; }  \
        else      { ++failed; std::cout << "  [FAIL] " << msg << "\n"; }  \
    } while (0)

int main() {
    std::cout << "=== TIVIDY: OBSERVER TESTS ===\n";

    ActivityDefinition def("submit", "Submit Proposal", "proposer");
    WorkItem item("Claim-1001", &def);

    EventBus bus;
    AuditObserver audit;
    NotificationObserver notifier("STATE_CHANGED");

    // 1. No bus attached: setState must still work and nothing is recorded
    item.setState("RUNNING");
    CHECK(item.getState() == "RUNNING", "item works with no bus attached");
    CHECK(audit.getLog().empty(), "no events recorded before the item has a bus");

    // 2. Attach bus + observers
    item.setEventBus(&bus);
    bus.attach(&audit);
    bus.attach(&notifier);
    bus.attach(&audit);  // duplicate attach must be ignored
    CHECK(bus.observerCount() == 2, "duplicate attach is ignored (2 observers)");

    // 3. One state change reaches BOTH observers
    item.setState("SUCCESS");
    CHECK(audit.getLog().size() == 1, "audit observer received the event");
    CHECK(notifier.getNotifications().size() == 1, "notification observer received the event");
    CHECK(audit.getLog()[0].find("RUNNING -> SUCCESS") != std::string::npos,
          "event payload shows old -> new state");
    CHECK(audit.getLog()[0].find("instance=Claim-1001") != std::string::npos,
          "event carries the instance id");
    CHECK(item.getInstanceId() == "Claim-1001", "WorkItem exposes its instance id");

    // 4. Detach: the detached observer stops hearing events
    bus.detach(&notifier);
    CHECK(bus.observerCount() == 1, "detach removes the observer");
    item.setState("FAILED");
    CHECK(audit.getLog().size() == 2, "audit still receives events after the other observer detached");
    CHECK(notifier.getNotifications().size() == 1, "detached observer receives nothing new");

    // 5. Filtering: a NotificationObserver only reacts to the event it watches
    NotificationObserver other("ASSIGNED");
    bus.attach(&other);
    item.setState("IDLE");  // STATE_CHANGED, ignored by 'other'
    CHECK(other.getNotifications().empty(), "observer ignores events it does not watch");

    // 6. Ordering
    std::vector<std::string> log = audit.getLog();
    CHECK(log.size() == 3 && log[0].find("SUCCESS") != std::string::npos &&
          log[1].find("FAILED") != std::string::npos &&
          log[2].find("-> IDLE") != std::string::npos,
          "audit log keeps events in the order they happened");

    // 7. An observer may detach itself while being notified (bus iterates over a copy)
    struct SelfDetaching : WorkFlowObserver {
        EventBus* b; int calls;
        SelfDetaching(EventBus* bus) : b(bus), calls(0) {}
        void update(const WorkFlowEvent&) override { ++calls; b->detach(this); }
    } selfDetaching(&bus);
    bus.attach(&selfDetaching);
    item.setState("RUNNING");
    item.setState("SUCCESS");
    CHECK(selfDetaching.calls == 1, "observer can detach itself safely during notify");

    audit.printLog();
    std::cout << "\n=== RESULT: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
