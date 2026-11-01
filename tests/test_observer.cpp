/**
 * @file test_observer.cpp
 * @brief GoogleTest unit tests for the Observer pattern (EventBus, AuditObserver, NotificationObserver).
 */
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "ActivityDefinition.h"
#include "AssignCommand.h"
#include "AuditObserver.h"
#include "EventBus.h"
#include "NotificationObserver.h"
#include "Participant.h"
#include "WorkItem.h"

namespace {

/** Remembers every event it receives so a test can inspect them. */
struct RecordingObserver : WorkFlowObserver {
    std::vector<WorkFlowEvent> events;
    void update(const WorkFlowEvent& event) override { events.push_back(event); }
};

/** Detaches itself from the bus the first time it is notified. */
struct SelfDetachingObserver : WorkFlowObserver {
    EventBus* bus;
    int calls;
    explicit SelfDetachingObserver(EventBus* b) : bus(b), calls(0) {}
    void update(const WorkFlowEvent&) override {
        ++calls;
        bus->detach(this);
    }
};

}  // namespace

//  EventBus bookkeeping 

class EventBusTest : public ::testing::Test {
protected:
    EventBus bus;
    AuditObserver audit;
    NotificationObserver notifier{"STATE_CHANGED"};
};

TEST_F(EventBusTest, StartsWithNoObservers) {
    EXPECT_EQ(bus.observerCount(), 0);
}

TEST_F(EventBusTest, AttachAddsObservers) {
    bus.attach(&audit);
    bus.attach(&notifier);
    EXPECT_EQ(bus.observerCount(), 2);
}

TEST_F(EventBusTest, AttachingTheSameObserverTwiceIsIgnored) {
    bus.attach(&audit);
    bus.attach(&audit);
    EXPECT_EQ(bus.observerCount(), 1);
}

TEST_F(EventBusTest, AttachingNullptrIsIgnored) {
    bus.attach(nullptr);
    EXPECT_EQ(bus.observerCount(), 0);
}

TEST_F(EventBusTest, DetachRemovesOnlyThatObserver) {
    bus.attach(&audit);
    bus.attach(&notifier);
    bus.detach(&audit);
    EXPECT_EQ(bus.observerCount(), 1);
}

TEST_F(EventBusTest, DetachingAnUnknownObserverIsHarmless) {
    bus.attach(&audit);
    bus.detach(&notifier);
    EXPECT_EQ(bus.observerCount(), 1);
}

TEST_F(EventBusTest, NotifyingWithNoObserversDoesNothing) {
    bus.notify(WorkFlowEvent("TEST", "i1", "w1", "nobody listening"));
    EXPECT_EQ(bus.observerCount(), 0);
}

TEST_F(EventBusTest, ObserverMayDetachItselfWhileBeingNotified) {
    SelfDetachingObserver observer(&bus);
    bus.attach(&observer);
    bus.notify(WorkFlowEvent("TEST", "i1", "w1", "first"));
    bus.notify(WorkFlowEvent("TEST", "i1", "w1", "second"));
    EXPECT_EQ(observer.calls, 1);
    EXPECT_EQ(bus.observerCount(), 0);
}

TEST_F(EventBusTest, SequenceNumbersIncreaseForEachNewEvent) {
    RecordingObserver recorder;
    bus.attach(&recorder);
    bus.notify(WorkFlowEvent("TEST", "i1", "w1", "a"));
    bus.notify(WorkFlowEvent("TEST", "i1", "w1", "b"));
    ASSERT_EQ(recorder.events.size(), 2u);
    EXPECT_GT(recorder.events[1].getSequence(), recorder.events[0].getSequence());
}

//  Work items publish their state changes 

class WorkItemEventsTest : public ::testing::Test {
protected:
    ActivityDefinition def{"submit", "Submit Proposal", "proposer"};
    WorkItem item{"Claim-1001", &def};
    EventBus bus;
    AuditObserver audit;
    NotificationObserver notifier{"STATE_CHANGED"};

    void connect() {
        item.setEventBus(&bus);
        bus.attach(&audit);
        bus.attach(&notifier);
    }
};

TEST_F(WorkItemEventsTest, WithoutABusTheStateStillChangesAndNothingIsPublished) {
    bus.attach(&audit);  // attached to the bus, but the item has no bus
    item.setState("Available");
    EXPECT_EQ(item.getState(), "Available");
    EXPECT_TRUE(audit.getLog().empty());
}

TEST_F(WorkItemEventsTest, OneChangeReachesEveryObserver) {
    connect();
    item.setState("Available");
    EXPECT_EQ(audit.getLog().size(), 1u);
    EXPECT_EQ(notifier.getNotifications().size(), 1u);
}

TEST_F(WorkItemEventsTest, EventSaysWhatChangedAndForWhichInstance) {
    connect();
    item.setState("Available");
    ASSERT_EQ(audit.getLog().size(), 1u);
    const std::string line = audit.getLog()[0];
    EXPECT_NE(line.find("Created -> Available"), std::string::npos);
    EXPECT_NE(line.find("instance=Claim-1001"), std::string::npos);
    EXPECT_NE(line.find("item=submit"), std::string::npos);
}

TEST_F(WorkItemEventsTest, EventsArriveInTheOrderTheyHappened) {
    connect();
    item.setState("Available");
    item.setState("Assigned");
    item.setState("In_Progress");
    std::vector<std::string> log = audit.getLog();
    ASSERT_EQ(log.size(), 3u);
    EXPECT_NE(log[0].find("Created -> Available"), std::string::npos);
    EXPECT_NE(log[1].find("Available -> Assigned"), std::string::npos);
    EXPECT_NE(log[2].find("Assigned -> In_Progress"), std::string::npos);
}

TEST_F(WorkItemEventsTest, DetachedObserverStopsHearingWhileOthersKeepHearing) {
    connect();
    bus.detach(&notifier);
    item.setState("Available");
    EXPECT_TRUE(notifier.getNotifications().empty());
    EXPECT_EQ(audit.getLog().size(), 1u);
}

TEST_F(WorkItemEventsTest, NotificationObserverIgnoresEventsItDoesNotWatch) {
    connect();
    NotificationObserver other("SOMETHING_ELSE");
    bus.attach(&other);
    item.setState("Available");
    EXPECT_TRUE(other.getNotifications().empty());
}

TEST_F(WorkItemEventsTest, InvalidOperationsPublishNothing) {
    connect();
    // A brand new item is Created: it cannot be started or completed yet.
    EXPECT_FALSE(item.start());
    EXPECT_FALSE(item.complete());
    EXPECT_EQ(item.getState(), "Created");
    EXPECT_TRUE(audit.getLog().empty());
}

TEST_F(WorkItemEventsTest, AssigningThroughTheCommandPublishesEachStep) {
    connect();
    std::vector<std::string> roles(1, "proposer");
    Participant dana("p1", "Dana", roles, 1);

    item.makeAvailable();  // Created -> Available
    AssignCommand command(&item, &dana);
    EXPECT_TRUE(command.execute());  // Available -> Assigned

    std::vector<std::string> log = audit.getLog();
    ASSERT_EQ(log.size(), 2u);
    EXPECT_NE(log[0].find("Created -> Available"), std::string::npos);
    EXPECT_NE(log[1].find("Available -> Assigned"), std::string::npos);
}
