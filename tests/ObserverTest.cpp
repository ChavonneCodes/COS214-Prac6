
/**
 * @file ObserverTest.cpp
 * @brief GoogleTest functionality tests for the Observer pattern.
 *
 * Tests EventBus, observers, and WorkItem state-change events.
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "WorkItem.h"
#include "ActivityDefinition.h"
#include "EventBus.h"
#include "AuditObserver.h"
#include "NotificationObserver.h"

// Test fixture
class ObserverTest : public ::testing::Test {
protected:
    ActivityDefinition def{"submit", "Submit Proposal", "proposer"};
    WorkItem item{"Claim-1001", &def};

    EventBus bus;
    AuditObserver audit;
    NotificationObserver notifier{"STATE_CHANGED"};

    void SetUp() override {
        // Attach the EventBus and observers before each test
        item.setEventBus(&bus);
        bus.attach(&audit);
        bus.attach(&notifier);
    }
};

// 1. WorkItem can change state without an EventBus attached
TEST(ObserverWithoutEventBusTest, StateChangeStillWorks) {
    ActivityDefinition def("submit", "Submit Proposal", "proposer");
    WorkItem item("Claim-1001", &def);

    item.setState("RUNNING");

    EXPECT_EQ(item.getState(), "RUNNING");
}

// 2. Duplicate observer attachment is ignored
TEST_F(ObserverTest, DuplicateObserverAttachmentIsIgnored) {
    bus.attach(&audit);

    EXPECT_EQ(bus.observerCount(), 2);
}

// 3. A state change reaches both observers
TEST_F(ObserverTest, StateChangeNotifiesBothObservers) {
    item.setState("SUCCESS");

    EXPECT_EQ(audit.getLog().size(), 1);
    EXPECT_EQ(notifier.getNotifications().size(), 1);
}

// 4. Event payload contains the previous and new states
TEST_F(ObserverTest, EventContainsOldAndNewStates) {
    item.setState("RUNNING");
    item.setState("SUCCESS");

    ASSERT_EQ(audit.getLog().size(), 2);

    EXPECT_NE(
        audit.getLog()[1].find("RUNNING -> SUCCESS"),
        std::string::npos
    );
}

// 5. Event contains the WorkItem instance ID
TEST_F(ObserverTest, EventContainsInstanceId) {
    item.setState("SUCCESS");

    ASSERT_EQ(audit.getLog().size(), 1);

    EXPECT_NE(
        audit.getLog()[0].find("instance=Claim-1001"),
        std::string::npos
    );

    EXPECT_EQ(item.getInstanceId(), "Claim-1001");
}

// 6. Detaching an observer prevents further notifications
TEST_F(ObserverTest, DetachedObserverReceivesNoNewEvents) {
    bus.detach(&notifier);

    EXPECT_EQ(bus.observerCount(), 1);

    item.setState("FAILED");

    EXPECT_EQ(audit.getLog().size(), 1);
    EXPECT_TRUE(notifier.getNotifications().empty());
}

// 7. NotificationObserver only reacts to the event it watches
TEST_F(ObserverTest, ObserverIgnoresUnwatchedEvents) {
    NotificationObserver other("ASSIGNED");
    bus.attach(&other);

    item.setState("IDLE");

    EXPECT_TRUE(other.getNotifications().empty());
}

// 8. Audit log preserves event ordering
TEST_F(ObserverTest, AuditLogPreservesEventOrder) {
    item.setState("SUCCESS");
    item.setState("FAILED");
    item.setState("IDLE");

    ASSERT_EQ(audit.getLog().size(), 3);

    EXPECT_NE(
        audit.getLog()[0].find("-> SUCCESS"),
        std::string::npos
    );

    EXPECT_NE(
        audit.getLog()[1].find("SUCCESS -> FAILED"),
        std::string::npos
    );

    EXPECT_NE(
        audit.getLog()[2].find("FAILED -> IDLE"),
        std::string::npos
    );
}

// 9. Observer can detach itself safely during notification
TEST_F(ObserverTest, ObserverCanDetachItselfDuringNotification) {
    class SelfDetachingObserver : public WorkFlowObserver {
    public:
        EventBus* bus;
        int calls;

        explicit SelfDetachingObserver(EventBus* eventBus)
            : bus(eventBus), calls(0) {}

        void update(const WorkFlowEvent&) override {
            ++calls;
            bus->detach(this);
        }
    };

    SelfDetachingObserver selfDetaching(&bus);

    bus.attach(&selfDetaching);

    item.setState("RUNNING");
    item.setState("SUCCESS");

    EXPECT_EQ(selfDetaching.calls, 1);
}
