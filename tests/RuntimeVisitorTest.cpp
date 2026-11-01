
#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "RuntimeVisitor.h"
#include "ProgressReportVisitor.h"
#include "WorkloadVisitor.h"
#include "StageDefinition.h"
#include "ActivityDefinition.h"
#include "SubProcessDefinition.h"
#include "StageInstance.h"
#include "SubProcessInstance.h"
#include "Participant.h"
#include "WorkItem.h"
#include "LabelledRuntimeDecorator.h"
#include "WorkflowEngine.h"


// ============================================================
// RECORDING VISITOR
// Records every visit so that traversal order can be tested.
// ============================================================

class RecordingVisitor : public RuntimeVisitor {
public:

    std::vector<std::string> calls;

    void visitWorkItem(WorkItem& item) override {
        calls.push_back("item:" + item.getId());
    }

    void visitStage(StageInstance& stage) override {
        calls.push_back("enter:" + stage.getId());
    }

    void visitSubProcess(SubProcessInstance& sub) override {
        calls.push_back("sub:" + sub.getId());
    }

    void leaveStage(StageInstance& stage) override {
        calls.push_back("leave:" + stage.getId());
    }
};


// ============================================================
// TEST FIXTURE
// Creates a reusable workflow tree for the test cases.
// ============================================================

class RuntimeVisitorTest : public ::testing::Test {

protected:

    // Definitions
    StageDefinition rootDef{"root", "Approval"};
    StageDefinition reviewDef{"review", "Review Stage"};

    ActivityDefinition submitDef{"submit", "Submit", "proposer"};
    ActivityDefinition checkDef{"check", "Check", "reviewer"};
    ActivityDefinition approveDef{"approve", "Approve", "manager"};

    SubProcessDefinition creditDef{"credit", "credit-check", 1};

    // Participants
    Participant ann{"p1", "Ann", {"proposer"}};
    Participant bob{"p2", "Bob", {"reviewer"}};

    // Runtime objects
    StageInstance* root = nullptr;

    WorkItem* submit = nullptr;
    StageInstance* review = nullptr;
    WorkItem* checkItem = nullptr;
    WorkItem* approve = nullptr;

    // Decorator
    RuntimeComponent* labelled = nullptr;


    // --------------------------------------------------------
    // Setup: runs before every test.
    // --------------------------------------------------------

    void SetUp() override {

        // Create root stage
        root = new StageInstance("case-1", &rootDef);

        // Create work items and nested stage
        submit = new WorkItem("case-1", &submitDef);

        review = new StageInstance("case-1", &reviewDef);

        checkItem = new WorkItem("case-1", &checkDef);

        approve = new WorkItem("case-1", &approveDef);


        // Construct workflow tree
        root->addChild(submit);

        review->addChild(checkItem);

        review->addChild(
            new SubProcessInstance("case-1", &creditDef)
        );

        root->addChild(review);

        root->addChild(approve);


        // Assign participants and change states
        submit->assignTo(&ann);
        submit->start();
        submit->complete();

        checkItem->assignTo(&bob);
        checkItem->start();

        // Approve remains Created and unassigned.


        // Wrap the root in a decorator.
        // The decorator now owns the root.
        labelled = new LabelledRuntimeDecorator(root, "Case 1");
    }


    // --------------------------------------------------------
    // Teardown: runs after every test.
    // --------------------------------------------------------

    void TearDown() override {

        // Deleting the decorator also deletes the wrapped tree.
        delete labelled;

        labelled = nullptr;
        root = nullptr;
    }


    // Expected visitor traversal order.
    std::vector<std::string> expectedCalls() {

        return {
            "enter:root",
            "item:submit",
            "enter:review",
            "item:check",
            "sub:credit",
            "leave:review",
            "item:approve",
            "leave:root"
        };
    }
};


// ============================================================
// TEST 1: DOUBLE DISPATCH AND TRAVERSAL ORDER
// ============================================================

TEST_F(RuntimeVisitorTest, VisitsEveryNodeInCorrectTreeOrder) {

    RecordingVisitor recorder;

    root->accept(recorder);

    EXPECT_EQ(recorder.calls, expectedCalls());
}


// ============================================================
// TEST 2: VISITOR THROUGH DECORATOR
// ============================================================

TEST_F(RuntimeVisitorTest, DecoratorForwardsVisitorCorrectly) {

    RecordingVisitor recorder;

    labelled->accept(recorder);

    EXPECT_EQ(recorder.calls, expectedCalls());
}


// ============================================================
// TEST 3: PROGRESS REPORT COUNTS
// ============================================================

TEST_F(RuntimeVisitorTest, CountsTotalAndCompletedWorkItems) {

    ProgressReportVisitor progress;

    labelled->accept(progress);

    EXPECT_EQ(progress.getTotalItems(), 3);

    EXPECT_EQ(progress.getCompletedItems(), 1);

    EXPECT_EQ(progress.getPercentComplete(), 33);

    EXPECT_FALSE(progress.isComplete());
}


// ============================================================
// TEST 4: PROGRESS REPORT STATE COUNTS
// ============================================================

TEST_F(RuntimeVisitorTest, CountsWorkItemsByState) {

    ProgressReportVisitor progress;

    labelled->accept(progress);

    std::map<std::string, int> states =
        progress.getStateCounts();

    EXPECT_EQ(states["Completed"], 1);

    EXPECT_EQ(states["In_Progress"], 1);

    EXPECT_EQ(states["Created"], 1);
}


// ============================================================
// TEST 5: PROGRESS REPORT CONTENT
// ============================================================

TEST_F(RuntimeVisitorTest, GeneratesCorrectProgressReport) {

    ProgressReportVisitor progress;

    labelled->accept(progress);

    std::vector<std::string> report =
        progress.getReport();

    ASSERT_EQ(report.size(), 7);

    EXPECT_EQ(
        report[3],
        "    [In_Progress] Check (Bob)"
    );

    EXPECT_EQ(
        report[5],
        "  [Created] Approve (unassigned)"
    );

    EXPECT_EQ(
        report.back(),
        "1/3 work items completed (33%)"
    );
}


// ============================================================
// TEST 6: COMPLETED WORKFLOW
// ============================================================

TEST_F(RuntimeVisitorTest, RecognisesWhenWorkflowIsComplete) {

    // Complete the remaining work items.
    checkItem->complete();

    approve->assignTo(&ann);
    approve->start();
    approve->complete();


    ProgressReportVisitor finished;

    labelled->accept(finished);


    EXPECT_EQ(finished.getTotalItems(), 3);

    EXPECT_EQ(finished.getCompletedItems(), 3);

    EXPECT_EQ(finished.getPercentComplete(), 100);

    EXPECT_TRUE(finished.isComplete());
}


// ============================================================
// TEST 7: EMPTY WORKFLOW
// ============================================================

TEST(RuntimeVisitorEmptyTest, HandlesWorkflowWithNoWorkItems) {

    ProgressReportVisitor empty;

    EXPECT_EQ(empty.getTotalItems(), 0);

    EXPECT_EQ(empty.getCompletedItems(), 0);

    EXPECT_EQ(empty.getPercentComplete(), 0);

    EXPECT_FALSE(empty.isComplete());
}


// ============================================================
// TEST 8: PARTICIPANT ORDER
// ============================================================

TEST_F(RuntimeVisitorTest, ListsParticipantsInOrderFirstEncountered) {

    // Complete the remaining items to match the original test.
    checkItem->complete();

    approve->assignTo(&ann);
    approve->start();
    approve->complete();


    WorkloadVisitor workload;

    labelled->accept(workload);

    std::vector<std::string> expected = {
        "Ann",
        "Bob"
    };

    EXPECT_EQ(workload.getParticipants(), expected);
}


// ============================================================
// TEST 9: ITEMS GROUPED BY PARTICIPANT
// ============================================================

TEST_F(RuntimeVisitorTest, GroupsWorkItemsByParticipant) {

    checkItem->complete();

    approve->assignTo(&ann);
    approve->start();
    approve->complete();


    WorkloadVisitor workload;

    labelled->accept(workload);


    std::vector<std::string> annItems = {
        "Submit",
        "Approve"
    };

    std::vector<std::string> bobItems = {
        "Check"
    };


    EXPECT_EQ(workload.getItemsFor("Ann"), annItems);

    EXPECT_EQ(workload.getItemsFor("Bob"), bobItems);
}


// ============================================================
// TEST 10: OPEN WORK ITEMS
// ============================================================

TEST_F(RuntimeVisitorTest, DoesNotCountCompletedItemsAsOpen) {

    checkItem->complete();

    approve->assignTo(&ann);
    approve->start();
    approve->complete();


    WorkloadVisitor workload;

    labelled->accept(workload);


    EXPECT_EQ(workload.getOpenCount("Ann"), 0);

    EXPECT_EQ(workload.getOpenCount("Bob"), 0);
}


// ============================================================
// TEST 11: UNKNOWN PARTICIPANT
// ============================================================

TEST_F(RuntimeVisitorTest, UnknownParticipantHasNoWorkItems) {

    WorkloadVisitor workload;

    labelled->accept(workload);


    EXPECT_TRUE(workload.getItemsFor("Zed").empty());

    EXPECT_EQ(workload.getOpenCount("Zed"), 0);
}


// ============================================================
// TEST 12: WORKFLOW ENGINE FACADE
// ============================================================

TEST(RuntimeVisitorFacadeTest, RejectsUnknownWorkflowInstance) {

    WorkflowEngine engine;

    ProgressReportVisitor visitor;

    EXPECT_FALSE(
        engine.applyVisitor("missing", visitor)
    );

    EXPECT_EQ(visitor.getTotalItems(), 0);
}
