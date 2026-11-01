/**
 * @file teststateAndcommand.cpp
 * @brief GoogleTest unit tests for the State and Command patterns.
 *testing for two patterns combined as state changes are encapsulated as commands
 * State:   valid and invalid transitions of a work item.
 * Command: execute, undo, and what the engine records.
 */
#include <gtest/gtest.h>
#include <sstream>
#include "WorkItem.h"
#include "ActivityDefinition.h"
#include "Participant.h"
#include "makeAvailableCommand.h"
#include "AssignCommand.h"
#include "StartCommand.h"
#include "CompleteCommand.h"
#include "RejectCommand.h"
#include "FailCommand.h"
#include "EscalateCommand.h"
#include "CancelCommand.h"
#include "WorkflowEngine.h"         
#include "HistoryManager.h"
#include "EventBus.h"
#include "AuditObserver.h"
#include "InMemoryDefinitionRepository.h"


namespace {
struct basicTestingTemplate {
    ActivityDefinition def{"review", "Review", "user"};
    WorkItem item{"inst", &def};
    Participant ann{"p1", "Ann", {"user"}};
};
// Moves the item forward using valid commands.
void toAvailable(basicTestingTemplate& s)  { makeAvailableCommand(&s.item).execute(); }
void toAssigned(basicTestingTemplate& s)   { toAvailable(s); AssignCommand(&s.item, &s.ann).execute(); }
void toInProgress(basicTestingTemplate& s) { toAssigned(s);  StartCommand(&s.item).execute(); }
void toCompleted(basicTestingTemplate& s)  { toInProgress(s); CompleteCommand(&s.item).execute(); }
void toRejected(basicTestingTemplate& s)   { toAssigned(s);  RejectCommand(&s.item).execute(); }
void toFailed(basicTestingTemplate& s)     { toInProgress(s); FailCommand(&s.item).execute(); }
void toEscalated(basicTestingTemplate& s)  { toAssigned(s);  EscalateCommand(&s.item).execute(); }
void toCancelled(basicTestingTemplate& s)  { CancelCommand(&s.item).execute(); }

//mock engine used to test 
struct workflowEngineTemplate : basicTestingTemplate {
    EventBus bus;
    HistoryManager history;
    AuditObserver audit;
    InMemoryDefinitionRepository repo;
    WorkflowEngine engine{&repo, &bus, &history, &audit};
    workflowEngineTemplate() { item.setEventBus(&bus); bus.attach(&audit); }
};
//invalid transitions
struct TransitionStruct {
    std::stringstream sink;
    std::streambuf* old;
    TransitionStruct()  : old(std::cout.rdbuf(sink.rdbuf())) {}
    ~TransitionStruct() { std::cout.rdbuf(old); }
};
}

//These are testing valid state transitions invoked through commands
//because commands return type is bool use ExpectTrue to determine if a wrokItem successfully changes its state
TEST(State, NewItemStartsCreated) {
    basicTestingTemplate s;
    EXPECT_EQ(s.item.getState(), "Created");
}

TEST(State, CreatedToAvailable) {
    basicTestingTemplate s;
    EXPECT_TRUE(makeAvailableCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Available");
}

TEST(State, AvailableToAssigned) {
    basicTestingTemplate s;
    toAvailable(s);
    EXPECT_TRUE(AssignCommand(&s.item, &s.ann).execute());
    EXPECT_EQ(s.item.getState(), "Assigned");
}

TEST(State, AssignedToInProgress) {
    basicTestingTemplate s;
    toAssigned(s);
    EXPECT_TRUE(StartCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "In_Progress");
}

TEST(State, InProgressToCompleted) {
    basicTestingTemplate s;
    toInProgress(s);
    EXPECT_TRUE(CompleteCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Completed");
}

TEST(State, AssignedToRejected) {
    basicTestingTemplate s;
    toAssigned(s);
    EXPECT_TRUE(RejectCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Rejected");
}

TEST(State, InProgressToFailed) {
    basicTestingTemplate s;
    toInProgress(s);
    EXPECT_TRUE(FailCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Failed");
}

TEST(State, AssignedToEscalated) {
    basicTestingTemplate s;
    toAssigned(s);
    EXPECT_TRUE(EscalateCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Escalated");
}

TEST(State, EscalatedBackToAssigned) {
    basicTestingTemplate s;
    toEscalated(s);
    EXPECT_TRUE(AssignCommand(&s.item, &s.ann).execute());
    EXPECT_EQ(s.item.getState(), "Assigned");
}

TEST(State, RejectedBackToAvailable) {
    basicTestingTemplate s;
    toRejected(s);
    EXPECT_TRUE(makeAvailableCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Available");
}

TEST(State, FailedBackToAvailable) {
    basicTestingTemplate s;
    toFailed(s);
    EXPECT_TRUE(makeAvailableCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Available");
}

TEST(State, CreatedToCancelled) {
    basicTestingTemplate s;
    EXPECT_TRUE(CancelCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Cancelled");
}

TEST(State, InProgressToCancelled) {
    basicTestingTemplate s;
    toInProgress(s);
    EXPECT_TRUE(CancelCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Cancelled");
}

//Testing invalid state transitions
//State pattern- states determine where they are allowed to go to next base class should return false

TEST(State, CreatedCannotStart) {
    basicTestingTemplate s;
    TransitionStruct q;
    EXPECT_FALSE(StartCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Created");
}

TEST(State, CreatedCannotComplete) {
    basicTestingTemplate s;
    TransitionStruct q;
    EXPECT_FALSE(CompleteCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Created");
}

TEST(State, AvailableCannotStartWithoutAnOwner) {
    basicTestingTemplate s;
    toAvailable(s);
    TransitionStruct q;
    EXPECT_FALSE(StartCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Available");
}

TEST(State, AssignedCannotCompleteWithoutStarting) {
    basicTestingTemplate s;
    toAssigned(s);
    TransitionStruct q;
    EXPECT_FALSE(CompleteCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Assigned");
}

TEST(State, InProgressCannotStartAgain) {
    basicTestingTemplate s;
    toInProgress(s);
    TransitionStruct q;
    EXPECT_FALSE(StartCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "In_Progress");
}

TEST(State, RejectedCannotComplete) {
    basicTestingTemplate s;
    toRejected(s);
    TransitionStruct q;
    EXPECT_FALSE(CompleteCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Rejected");
}

TEST(State, FailedCannotComplete) {
    basicTestingTemplate s;
    toFailed(s);
    TransitionStruct q;
    EXPECT_FALSE(CompleteCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Failed");
}
TEST(State, EscalatedCannotStartUntilReassigned) {
    basicTestingTemplate s;
    toEscalated(s);
    TransitionStruct q;
    EXPECT_FALSE(StartCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Escalated");
}
TEST(State, CompletedIsFinal) {
    basicTestingTemplate s;
    toCompleted(s);
    TransitionStruct q;
    EXPECT_FALSE(StartCommand(&s.item).execute());
    EXPECT_FALSE(CancelCommand(&s.item).execute());
    EXPECT_FALSE(makeAvailableCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Completed");
}
TEST(State, CancelledIsFinal) {
    basicTestingTemplate s;
    toCancelled(s);
    TransitionStruct q;
    EXPECT_FALSE(makeAvailableCommand(&s.item).execute());
    EXPECT_FALSE(StartCommand(&s.item).execute());
    EXPECT_FALSE(CancelCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Cancelled");
}
TEST(State, InvalidTransitionChangesNothingElse) {
    basicTestingTemplate s;
    toAssigned(s);
    TransitionStruct q;
    EXPECT_FALSE(CompleteCommand(&s.item).execute());
    EXPECT_EQ(s.item.getState(), "Assigned");
    EXPECT_EQ(s.item.getAssignedParticipant(), &s.ann);   
    EXPECT_EQ(s.ann.getOpenItems(), 1);                   
}


//Testing command submission using engine, execute, undo

TEST(Command, ExecuteReturnsTrueWhenTheStateAllowsIt) {
    basicTestingTemplate s;
    toAssigned(s);
    StartCommand cmd(&s.item);
    EXPECT_TRUE(cmd.execute());
}

TEST(Command, ExecuteReturnsFalseWhenTheStateRefusesIt) {
    basicTestingTemplate s;
    TransitionStruct q;
    CompleteCommand cmd(&s.item);       // a Created item cannot complete
    EXPECT_FALSE(cmd.execute());
}

TEST(Command, ExecuteDoesNothingWithoutAReceiver) {
    TransitionStruct q;
    StartCommand cmd(nullptr);
    EXPECT_FALSE(cmd.execute());
}

TEST(Command, AssignNeedsAParticipant) {
    basicTestingTemplate s;
    toAvailable(s);
    TransitionStruct q;
    AssignCommand cmd(&s.item, nullptr);
    EXPECT_FALSE(cmd.execute());
    EXPECT_EQ(s.item.getState(), "Available");
}

TEST(Command, AssignBindsTheOwnerAndCountsTheItem) {
    basicTestingTemplate s;
    toAvailable(s);
    AssignCommand(&s.item, &s.ann).execute();
    EXPECT_EQ(s.item.getAssignedParticipant(), &s.ann);
    EXPECT_EQ(s.ann.getOpenItems(), 1);
}

TEST(Command, CompleteKeepsTheOwnerOnRecordButLowersTheWorkload) {
    basicTestingTemplate s;
    toInProgress(s);
    CompleteCommand(&s.item).execute();
    EXPECT_EQ(s.item.getAssignedParticipant(), &s.ann);
    EXPECT_EQ(s.ann.getOpenItems(), 0);
}

TEST(Command, EscalateHandsTheItemOver) {
    basicTestingTemplate s;
    toAssigned(s);
    EscalateCommand(&s.item).execute();
    EXPECT_EQ(s.item.getAssignedParticipant(), nullptr);
    EXPECT_EQ(s.ann.getOpenItems(), 0);
}

TEST(Command, DescribesItselfAndItsInstance) {
    basicTestingTemplate s;
    StartCommand cmd(&s.item);
    EXPECT_FALSE(cmd.getDescription().empty());
    EXPECT_EQ(cmd.getInstanceId(), "inst");
}


TEST(Command, UndoMakeAvailable) {
    TransitionStruct q;
    basicTestingTemplate s;
    makeAvailableCommand cmd(&s.item);
    cmd.execute();
    cmd.undo();
    EXPECT_EQ(s.item.getState(), "Created");
}

TEST(Command, UndoAssignRemovesTheOwnerAndTheWorkload) {
    TransitionStruct q;
    basicTestingTemplate s;
    toAvailable(s);
    AssignCommand cmd(&s.item, &s.ann);
    cmd.execute();
    cmd.undo();
    EXPECT_EQ(s.item.getState(), "Available");
    EXPECT_EQ(s.item.getAssignedParticipant(), nullptr);
    EXPECT_EQ(s.ann.getOpenItems(), 0);
}

TEST(Command, UndoStart) {
    TransitionStruct q;
    basicTestingTemplate s;
    toAssigned(s);
    StartCommand cmd(&s.item);
    cmd.execute();
    cmd.undo();
    EXPECT_EQ(s.item.getState(), "Assigned");
}

TEST(Command, UndoCompleteBringsTheWorkloadBack) {
    TransitionStruct q;
    basicTestingTemplate s;
    toInProgress(s);
    CompleteCommand cmd(&s.item);
    cmd.execute();
    ASSERT_EQ(s.ann.getOpenItems(), 0);
    cmd.undo();
    EXPECT_EQ(s.item.getState(), "In_Progress");
    EXPECT_EQ(s.item.getAssignedParticipant(), &s.ann);
    EXPECT_EQ(s.ann.getOpenItems(), 1);
}
TEST(Command, UndoCancelBringsTheWorkloadBack) {
    TransitionStruct q;
    basicTestingTemplate s;
    toAssigned(s);
    CancelCommand cmd(&s.item);
    cmd.execute();
    ASSERT_EQ(s.ann.getOpenItems(), 0);
    cmd.undo();
    EXPECT_EQ(s.item.getState(), "Assigned");
    EXPECT_EQ(s.ann.getOpenItems(), 1);
}

TEST(Command, UndoEscalateGivesTheItemBackToItsOwner) {
    TransitionStruct q;
    basicTestingTemplate s;
    toAssigned(s);
    EscalateCommand cmd(&s.item);
    cmd.execute();
    cmd.undo();
    EXPECT_EQ(s.item.getState(), "Assigned");
    EXPECT_EQ(s.item.getAssignedParticipant(), &s.ann);
    EXPECT_EQ(s.ann.getOpenItems(), 1);
}

TEST(Command, UndoRejectAndFail) {
    TransitionStruct q;
    basicTestingTemplate a;
    toAssigned(a);
    RejectCommand reject(&a.item);
    reject.execute();
    reject.undo();
    EXPECT_EQ(a.item.getState(), "Assigned");

    basicTestingTemplate b;
    toInProgress(b);
    FailCommand fail(&b.item);
    fail.execute();
    fail.undo();
    EXPECT_EQ(b.item.getState(), "In_Progress");
}
TEST(Command, UndoingInReverseOrderWalksBackThroughTheStates) {
    TransitionStruct q;
    basicTestingTemplate s;
    toAssigned(s);
    StartCommand start(&s.item);
    CompleteCommand complete(&s.item);
    start.execute();
    complete.execute();
    EXPECT_EQ(s.item.getState(), "Completed");
    complete.undo();
    EXPECT_EQ(s.item.getState(), "In_Progress");
    start.undo();
    EXPECT_EQ(s.item.getState(), "Assigned");
}

TEST(Command, ARefusedCommandLeavesTheItemExactlyAsItWas) {
    basicTestingTemplate s;
    toAssigned(s);
    TransitionStruct q;
    CompleteCommand cmd(&s.item);       // refused: Assigned cannot complete
    EXPECT_FALSE(cmd.execute());
    EXPECT_EQ(s.item.getState(), "Assigned");
    EXPECT_EQ(s.item.getAssignedParticipant(), &s.ann);
    EXPECT_EQ(s.ann.getOpenItems(), 1);
}

//Calling engine undoLast
//Does engine history and each states history connect accurately, are they cohesive??
TEST(Command, EngineRecordsASuccessfulCommand) {
    workflowEngineTemplate w;
    toAssigned(w);                      // set up with stack commands, so the history starts empty
    EXPECT_TRUE(w.engine.submit(new StartCommand(&w.item)));
    EXPECT_EQ(w.history.getAll().size(), 1u);
}

TEST(Command, EngineDoesNotRecordARefusedCommand) {
    workflowEngineTemplate w;
    TransitionStruct q;
    EXPECT_FALSE(w.engine.submit(new CompleteCommand(&w.item)));
    EXPECT_EQ(w.history.getAll().size(), 0u);
}

TEST(Command, UndoLastReversesTheMostRecentCommand) {
    TransitionStruct q;
    workflowEngineTemplate w;
    toAssigned(w);
    w.engine.submit(new StartCommand(&w.item));
    w.engine.undoLast();
    EXPECT_EQ(w.item.getState(), "Assigned");
}

TEST(Command, UndoLastTwiceGoesBackTwoSteps) {
    TransitionStruct q;
    workflowEngineTemplate w;
    toAssigned(w);
    w.engine.submit(new StartCommand(&w.item));
    w.engine.submit(new CompleteCommand(&w.item));
    w.engine.undoLast();
    EXPECT_EQ(w.item.getState(), "In_Progress");
    w.engine.undoLast();
    EXPECT_EQ(w.item.getState(), "Assigned");
}
TEST(Command, UndoLastWithNothingToUndoIssafe) {
    TransitionStruct q;
    workflowEngineTemplate w;
    EXPECT_NO_THROW(w.engine.undoLast());
    EXPECT_EQ(w.item.getState(), "Created");
}

TEST(Command, SuccessfulCommandAddsOneentryToAuditLog) {
    workflowEngineTemplate w;
    size_t before = w.audit.getLog().size();
    w.engine.submit(new makeAvailableCommand(&w.item));
    EXPECT_EQ(w.audit.getLog().size(), before + 1);
}
