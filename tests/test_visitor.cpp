/**
 * @file test_visitor.cpp
 * @brief GoogleTest unit tests for the Visitor pattern
 *        (RuntimeVisitor, ProgressReportVisitor, WorkloadVisitor).
 *
 * Covers double dispatch, traversal order, decorated trees, sub-process
 * children and the empty cases.
 */
#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ActivityDefinition.h"
#include "AssignCommand.h"
#include "CancelCommand.h"
#include "CompleteCommand.h"
#include "LabelledRuntimeDecorator.h"
#include "Participant.h"
#include "ProgressReportVisitor.h"
#include "RuntimeVisitor.h"
#include "StageDefinition.h"
#include "StageInstance.h"
#include "StartCommand.h"
#include "SubProcessDefinition.h"
#include "SubProcessInstance.h"
#include "WorkItem.h"
#include "WorkflowInstance.h"
#include "WorkloadVisitor.h"
#include "makeAvailableCommand.h"

namespace {

/** Records every call so a test can check double dispatch and traversal order. */
struct RecordingVisitor : RuntimeVisitor {
    std::vector<std::string> calls;
    void visitWorkItem(WorkItem& item) override { calls.push_back("item:" + item.getId()); }
    void visitStage(StageInstance& stage) override { calls.push_back("enter:" + stage.getId()); }
    void visitSubProcess(SubProcessInstance& sub) override { calls.push_back("sub:" + sub.getId()); }
    void leaveStage(StageInstance& stage) override { calls.push_back("leave:" + stage.getId()); }
};

std::vector<std::string> list(std::initializer_list<std::string> values) {
    return std::vector<std::string>(values);
}

void assign(WorkItem* item, Participant* person) {
    ASSERT_TRUE(makeAvailableCommand(item).execute());
    ASSERT_TRUE(AssignCommand(item, person).execute());
}

void finish(WorkItem* item, Participant* person) {
    assign(item, person);
    ASSERT_TRUE(StartCommand(item).execute());
    ASSERT_TRUE(CompleteCommand(item).execute());
}

}  // namespace

/**
 * Runtime tree: Approval -> [Submit, Review Stage -> [Check, credit], Approve].
 * The definitions are members because the runtime tree does not own them.
 */
class VisitorTest : public ::testing::Test {
protected:
    StageDefinition rootDef{"root", "Approval"};
    StageDefinition reviewDef{"review", "Review Stage"};
    ActivityDefinition submitDef{"submit", "Submit", "proposer"};
    ActivityDefinition checkDef{"check", "Check", "reviewer"};
    ActivityDefinition approveDef{"approve", "Approve", "manager"};
    SubProcessDefinition creditDef{"credit", "credit-check", 1};

    std::vector<std::string> annRoles{"proposer", "manager"};
    std::vector<std::string> bobRoles{"reviewer"};
    Participant ann{"p1", "Ann", annRoles};
    Participant bob{"p2", "Bob", bobRoles};

    std::unique_ptr<StageInstance> root;
    WorkItem* submit = nullptr;
    WorkItem* check = nullptr;
    WorkItem* approve = nullptr;
    SubProcessInstance* credit = nullptr;

    void SetUp() override {
        root.reset(new StageInstance("case-1", &rootDef));
        submit = new WorkItem("case-1", &submitDef);
        check = new WorkItem("case-1", &checkDef);
        approve = new WorkItem("case-1", &approveDef);
        credit = new SubProcessInstance("case-1", &creditDef);

        StageInstance* review = new StageInstance("case-1", &reviewDef);
        review->addChild(check);
        review->addChild(credit);
        root->addChild(submit);
        root->addChild(review);
        root->addChild(approve);
    }

    /** Submit is Completed, Check is In_Progress, Approve is untouched. */
    void partlyDone() {
        finish(submit, &ann);
        assign(check, &bob);
        ASSERT_TRUE(StartCommand(check).execute());
    }
};

// ---------------------------------------------------------------- double dispatch

TEST_F(VisitorTest, EachNodeCallsItsOwnVisitMethodInTreeOrder) {
    RecordingVisitor recorder;
    root->accept(recorder);
    EXPECT_EQ(recorder.calls, list({"enter:root", "item:submit", "enter:review", "item:check",
                                    "sub:credit", "leave:review", "item:approve", "leave:root"}));
}

TEST_F(VisitorTest, ADecoratedTreeIsVisitedExactlyLikeThePlainTree) {
    RecordingVisitor plain;
    root->accept(plain);

    LabelledRuntimeDecorator labelled(root.release(), "Case 1");  // now owns the tree
    RecordingVisitor decorated;
    labelled.accept(decorated);
    EXPECT_EQ(decorated.calls, plain.calls);
}

TEST_F(VisitorTest, ASingleWorkItemCanBeVisitedOnItsOwn) {
    RecordingVisitor recorder;
    submit->accept(recorder);
    EXPECT_EQ(recorder.calls, list({"item:submit"}));
}

// ---------------------------------------------------------------- ProgressReportVisitor

TEST(ProgressReportVisitorTest, WithNoWorkItemsItIsZeroPercentAndNotComplete) {
    ProgressReportVisitor empty;
    EXPECT_EQ(empty.getTotalItems(), 0);
    EXPECT_EQ(empty.getPercentComplete(), 0);
    EXPECT_FALSE(empty.isComplete());
    EXPECT_EQ(empty.getReport(), list({"0/0 work items completed (0%)"}));
}

TEST_F(VisitorTest, ProgressCountsItemsAndRoundsThePercentageDown) {
    partlyDone();
    ProgressReportVisitor progress;
    root->accept(progress);
    EXPECT_EQ(progress.getTotalItems(), 3);
    EXPECT_EQ(progress.getCompletedItems(), 1);
    EXPECT_EQ(progress.getPercentComplete(), 33);
    EXPECT_FALSE(progress.isComplete());
}

TEST_F(VisitorTest, ProgressCountsEachState) {
    partlyDone();
    ProgressReportVisitor progress;
    root->accept(progress);
    std::map<std::string, int> states = progress.getStateCounts();
    EXPECT_EQ(states["Completed"], 1);
    EXPECT_EQ(states["In_Progress"], 1);
    EXPECT_EQ(states["Created"], 1);
}

TEST_F(VisitorTest, ReportIndentsNestedNodesAndEndsWithTheSummary) {
    partlyDone();
    ProgressReportVisitor progress;
    root->accept(progress);
    EXPECT_EQ(progress.getReport(), list({"Stage: Approval",
                                          "  [Completed] Submit (Ann)",
                                          "  Stage: Review Stage",
                                          "    [In_Progress] Check (Bob)",
                                          "    [IDLE] sub-process credit",
                                          "  [Created] Approve (unassigned)",
                                          "1/3 work items completed (33%)"}));
}

TEST_F(VisitorTest, WhenEveryItemIsCompletedProgressIsComplete) {
    finish(submit, &ann);
    finish(check, &bob);
    finish(approve, &ann);
    ProgressReportVisitor progress;
    root->accept(progress);
    EXPECT_TRUE(progress.isComplete());
    EXPECT_EQ(progress.getPercentComplete(), 100);
}

TEST_F(VisitorTest, ACancelledItemCountsAsNotCompleted) {
    ASSERT_TRUE(CancelCommand(approve).execute());
    ProgressReportVisitor progress;
    root->accept(progress);
    EXPECT_EQ(progress.getStateCounts()["Cancelled"], 1);
    EXPECT_EQ(progress.getCompletedItems(), 0);
}

TEST_F(VisitorTest, ProgressOnlyReadsTheTree) {
    partlyDone();
    ProgressReportVisitor progress;
    root->accept(progress);
    EXPECT_EQ(submit->getState(), "Completed");
    EXPECT_EQ(check->getState(), "In_Progress");
    EXPECT_EQ(approve->getState(), "Created");
}

// ---------------------------------------------------------------- WorkloadVisitor

TEST(WorkloadVisitorTest, AnUnknownParticipantHasNoItems) {
    WorkloadVisitor workload;
    EXPECT_TRUE(workload.getParticipants().empty());
    EXPECT_TRUE(workload.getItemsFor("Zed").empty());
    EXPECT_EQ(workload.getOpenCount("Zed"), 0);
    EXPECT_TRUE(workload.getSummary().empty());
}

TEST_F(VisitorTest, WorkloadGroupsItemsByPersonInTheOrderFirstSeen) {
    finish(submit, &ann);
    finish(check, &bob);
    finish(approve, &ann);
    WorkloadVisitor workload;
    root->accept(workload);
    EXPECT_EQ(workload.getParticipants(), list({"Ann", "Bob"}));
    EXPECT_EQ(workload.getItemsFor("Ann"), list({"Submit", "Approve"}));
    EXPECT_EQ(workload.getItemsFor("Bob"), list({"Check"}));
}

TEST_F(VisitorTest, OnlyUnfinishedItemsCountAsOpen) {
    partlyDone();
    WorkloadVisitor workload;
    root->accept(workload);
    EXPECT_EQ(workload.getOpenCount("Ann"), 0);
    EXPECT_EQ(workload.getOpenCount("Bob"), 1);
}

TEST_F(VisitorTest, ACancelledItemIsNotOpen) {
    assign(check, &bob);
    ASSERT_TRUE(CancelCommand(check).execute());
    WorkloadVisitor workload;
    root->accept(workload);
    EXPECT_EQ(workload.getOpenCount("Bob"), 0);
}

TEST_F(VisitorTest, ItemsNobodyHoldsAreListedAsUnassigned) {
    partlyDone();
    WorkloadVisitor workload;
    root->accept(workload);
    EXPECT_EQ(workload.getUnassigned(), list({"Approve"}));
}

TEST_F(VisitorTest, SummaryHasOneLinePerPersonThenTheUnassignedCount) {
    partlyDone();
    WorkloadVisitor workload;
    root->accept(workload);
    EXPECT_EQ(workload.getSummary(), list({"Ann: 0 open of 1 (Submit)",
                                           "Bob: 1 open of 1 (Check)",
                                           "Unassigned: 1 item(s)"}));
}

// ---------------------------------------------------------------- sub-process children

/** credit calls a child workflow with one task, so the visitors must descend into it. */
class SubProcessVisitorTest : public VisitorTest {
protected:
    ActivityDefinition auditDef{"audit", "Audit", "reviewer"};
    WorkItem* audit = nullptr;
    std::unique_ptr<WorkflowInstance> child;

    void SetUp() override {
        VisitorTest::SetUp();
        audit = new WorkItem("case-1-credit", &auditDef);
        child.reset(new WorkflowInstance("case-1-credit", "credit-check", 1, audit, nullptr));
        credit->setChildInstance(child.get());
    }
};

TEST_F(SubProcessVisitorTest, ProgressCountsTheChildWorkflowsItems) {
    finish(audit, &bob);
    ProgressReportVisitor progress;
    root->accept(progress);
    EXPECT_EQ(progress.getTotalItems(), 4);
    EXPECT_EQ(progress.getCompletedItems(), 1);
}

TEST_F(SubProcessVisitorTest, ReportIndentsTheChildUnderItsSubProcess) {
    finish(audit, &bob);
    ProgressReportVisitor progress;
    root->accept(progress);
    std::vector<std::string> report = progress.getReport();
    ASSERT_EQ(report.size(), 8u);
    EXPECT_EQ(report[4], "    [IDLE] sub-process credit");
    EXPECT_EQ(report[5], "      [Completed] Audit (Bob)");
    EXPECT_EQ(report[6], "  [Created] Approve (unassigned)");
}

TEST_F(SubProcessVisitorTest, WorkloadCountsTheChildWorkflowsItems) {
    assign(audit, &bob);
    WorkloadVisitor workload;
    root->accept(workload);
    EXPECT_EQ(workload.getItemsFor("Bob"), list({"Audit"}));
    EXPECT_EQ(workload.getOpenCount("Bob"), 1);
}
