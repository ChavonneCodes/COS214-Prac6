
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "ActivityDefinition.h"
#include "WorkItem.h"
#include "Participant.h"
#include "WorkflowAction.h"

#include "AssignCommand.h"
#include "StartCommand.h"
#include "CompleteCommand.h"
#include "RejectCommand.h"
#include "CancelCommand.h"
#include "EscalateCommand.h"

#include "WorkflowEngine.h"
#include "InMemoryDefinitionRepository.h"
#include "EventBus.h"
#include "HistoryManager.h"
#include "NotificationObserver.h"

// Fixture shared by the tests.
class WorkflowCommandTest : public ::testing::Test {
protected:
    InMemoryDefinitionRepository repository;
    EventBus eventBus;
    HistoryManager history;
    AuditObserver audit;
    NotificationObserver decisions;

    WorkflowEngine engine;

    ActivityDefinition* definition;
    Participant* participant;

    std::vector<std::string> roles;

    WorkflowCommandTest()
        : decisions("APPROVAL_DECISION"),
          engine(&repository, &eventBus, &history, &audit),
          definition(nullptr),
          participant(nullptr) {
    }

    void SetUp() override {
        eventBus.attach(&audit);
        eventBus.attach(&decisions);

        definition = new ActivityDefinition(
            "act1", "Review", "role1"
        );

        roles.push_back("role1");

        participant = new Participant(
            "emp1", "Thabo", roles, 2
        );
    }

    void TearDown() override {
        delete participant;
        delete definition;
    }

    // Helper to check the current WorkItem state.
    void expectState(WorkItem& item, const std::string& expected) {
        ASSERT_NE(item.getCurrentState(), nullptr);

        EXPECT_EQ(
            item.getCurrentState()->getName(),
            expected
        );
    }
};

// --------------------------------------------------
// TEST 1: Initial WorkItem state
// --------------------------------------------------

TEST_F(WorkflowCommandTest, WorkItemStartsInCreatedState) {
    WorkItem item("item-A", definition);

    expectState(item, "Created");
}

// --------------------------------------------------
// TEST 2: Make WorkItem available
// --------------------------------------------------

TEST_F(WorkflowCommandTest, CreatedWorkItemCanBecomeAvailable) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    expectState(item, "Available");
}

// --------------------------------------------------
// TEST 3: Reject an invalid command
// --------------------------------------------------

TEST_F(WorkflowCommandTest, CannotCompleteWorkItemBeforeStartingIt) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    CompleteCommand complete(&item);

    engine.submit(&complete);

    // Completing an Available item should not change its state.
    expectState(item, "Available");
}

// --------------------------------------------------
// TEST 4: Assign a WorkItem
// --------------------------------------------------

TEST_F(WorkflowCommandTest, AssignCommandChangesStateToAssigned) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    AssignCommand assign(&item, participant);

    engine.submit(&assign);

    expectState(item, "Assigned");
}

// --------------------------------------------------
// TEST 5: Start a WorkItem
// --------------------------------------------------

TEST_F(WorkflowCommandTest, StartCommandChangesStateToInProgress) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    AssignCommand assign(&item, participant);
    engine.submit(&assign);

    StartCommand start(&item);
    engine.submit(&start);

    expectState(item, "In_Progress");
}

// --------------------------------------------------
// TEST 6: Complete a WorkItem
// --------------------------------------------------

TEST_F(WorkflowCommandTest, CompleteCommandChangesStateToCompleted) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    AssignCommand assign(&item, participant);
    engine.submit(&assign);

    StartCommand start(&item);
    engine.submit(&start);

    CompleteCommand complete(&item);
    engine.submit(&complete);

    expectState(item, "Completed");
}

// --------------------------------------------------
// TEST 7: Reject a WorkItem
// --------------------------------------------------

TEST_F(WorkflowCommandTest, RejectCommandChangesStateToRejected) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    AssignCommand assign(&item, participant);
    engine.submit(&assign);

    RejectCommand reject(&item);
    engine.submit(&reject);

    expectState(item, "Rejected");
}

// --------------------------------------------------
// TEST 8: Make a rejected WorkItem available again
// --------------------------------------------------

TEST_F(WorkflowCommandTest, RejectedWorkItemCanBecomeAvailableAgain) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    AssignCommand assign(&item, participant);
    engine.submit(&assign);

    RejectCommand reject(&item);
    engine.submit(&reject);

    expectState(item, "Rejected");

    item.makeAvailable();

    expectState(item, "Available");
}

// --------------------------------------------------
// TEST 9: Escalate a WorkItem
// --------------------------------------------------

TEST_F(WorkflowCommandTest, EscalateCommandChangesStateToEscalated) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    AssignCommand assign(&item, participant);
    engine.submit(&assign);

    EscalateCommand escalate(&item);

    engine.submit(&escalate);

    expectState(item, "Escalated");
}

// --------------------------------------------------
// TEST 10: Cancel a WorkItem
// --------------------------------------------------

TEST_F(WorkflowCommandTest, CancelCommandChangesStateToCancelled) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    AssignCommand assign(&item, participant);
    engine.submit(&assign);

    EscalateCommand escalate(&item);
    engine.submit(&escalate);

    CancelCommand cancel(&item);
    engine.submit(&cancel);

    expectState(item, "Cancelled");
}

// --------------------------------------------------
// TEST 11: Undo the last command
// --------------------------------------------------

TEST_F(WorkflowCommandTest, UndoLastReversesTheLastCommand) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    AssignCommand assign(&item, participant);
    engine.submit(&assign);

    StartCommand start(&item);
    engine.submit(&start);

    expectState(item, "In_Progress");

    engine.undoLast();

    // Undoing Start should restore the Assigned state.
    expectState(item, "Assigned");
}

// --------------------------------------------------
// TEST 12: Multiple undo operations
// --------------------------------------------------

TEST_F(WorkflowCommandTest, MultipleUndoOperationsRestorePreviousStates) {
    WorkItem item("item-A", definition);

    item.makeAvailable();

    AssignCommand assign(&item, participant);
    engine.submit(&assign);

    StartCommand start(&item);
    engine.submit(&start);

    CompleteCommand complete(&item);
    engine.submit(&complete);

    expectState(item, "Completed");

    engine.undoLast();
    expectState(item, "In_Progress");

    engine.undoLast();
    expectState(item, "Assigned");

    engine.undoLast();
    expectState(item, "Available");
}
