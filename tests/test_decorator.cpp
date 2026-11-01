/** @file test_decorator.cpp
 *  @brief GoogleTest coverage of runtime decorator policies and composition.
 *  Definitions and participants outlive the decorators; wrappers own runtime nodes.
 */
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <sstream>

#include "AllowedStatesRuntimeDecorator.h"
#include "LabelledRuntimeDecorator.h"
#include "LoggingRuntimeDecorator.h"
#include "Participant.h"
#include "PriorityRuntimeDecorator.h"
#include "RuntimeVisitor.h"
#include "SecurityRuntimeDecorator.h"
#include "StageDefinition.h"
#include "StageInstance.h"

namespace {
// Restore stdout even when an assertion returns early or an operation throws.
class DecoratorOutputCapture {
    std::ostringstream output;
    std::streambuf* previous;
public:
    DecoratorOutputCapture() : previous(std::cout.rdbuf(output.rdbuf())) {}
    ~DecoratorOutputCapture() { std::cout.rdbuf(previous); }
    std::string text() const { return output.str(); }
};

struct DecoratorStageVisitor : RuntimeVisitor {
    std::vector<std::string> calls;
    void visitWorkItem(WorkItem&) override {}
    void visitSubProcess(SubProcessInstance&) override {}
    void visitStage(StageInstance& stage) override { calls.push_back("enter:" + stage.getId()); }
    void leaveStage(StageInstance& stage) override { calls.push_back("leave:" + stage.getId()); }
};
}

TEST(RuntimeDecorator, RejectsNullComponent) {
    EXPECT_THROW({ RuntimeComponentDecorator wrapper(nullptr); }, std::invalid_argument);
}

TEST(SecurityDecorator, DeniesMissingAndWrongRoleWithoutChangingState) {
    StageDefinition definition("review", "Review");
    Participant guest("guest", "Guest", {"guest"});
    SecurityRuntimeDecorator stage(new StageInstance("case-1", &definition), "reviewer");
    EXPECT_FALSE(stage.canModify());
    EXPECT_THROW(stage.setState("RUNNING"), std::runtime_error);
    stage.setActor(&guest);
    EXPECT_FALSE(stage.canModify());
    EXPECT_THROW(stage.setState("RUNNING"), std::runtime_error);
    EXPECT_EQ(stage.getState(), "IDLE");
    EXPECT_EQ(stage.getId(), "review");  // Reads do not require authorisation.
}

TEST(SecurityDecorator, AuthorisesRoleAndRevokesAccessWhenActorIsRemoved) {
    StageDefinition definition("review", "Review");
    Participant reviewer("reviewer", "Reviewer", {"reviewer"});
    SecurityRuntimeDecorator stage(new StageInstance("case-1", &definition), "reviewer", &reviewer);
    EXPECT_TRUE(stage.canModify());
    stage.setState("RUNNING");
    EXPECT_EQ(stage.getState(), "RUNNING");
    stage.setActor(nullptr);
    EXPECT_FALSE(stage.canModify());
    EXPECT_THROW(stage.setState("COMPLETED"), std::runtime_error);
    EXPECT_EQ(stage.getState(), "RUNNING");
}

TEST(SecurityDecorator, DeniedChildInsertionRetainsCallerOwnership) {
    StageDefinition definition("review", "Review"), childDefinition("child", "Child");
    Participant reviewer("reviewer", "Reviewer", {"reviewer"});
    SecurityRuntimeDecorator stage(new StageInstance("case-1", &definition), "reviewer");
    std::unique_ptr<RuntimeComponent> child(new StageInstance("case-1", &childDefinition));
    EXPECT_THROW(stage.addChild(child.get()), std::runtime_error);
    EXPECT_TRUE(stage.getChildren().empty());
    EXPECT_EQ(child->getId(), "child");
    stage.setActor(&reviewer);
    stage.addChild(child.get());
    RuntimeComponent* inserted = child.release();
    ASSERT_EQ(stage.getChildren().size(), 1u);
    EXPECT_EQ(stage.getChildren().front(), inserted);
}

TEST(SecurityDecorator, RejectsEmptyRequiredRole) {
    StageDefinition definition("review", "Review");
    EXPECT_THROW({ SecurityRuntimeDecorator stage(new StageInstance("case-1", &definition), ""); },
                 std::invalid_argument);
}

TEST(AllowedStatesDecorator, RejectsUnknownAndWrongCaseNamesWithoutMutation) {
    StageDefinition definition("review", "Review");
    AllowedStatesRuntimeDecorator stage(new StageInstance("case-1", &definition), {"IDLE", "RUNNING"});
    stage.setState("RUNNING");
    EXPECT_THROW(stage.setState("BANANA"), std::invalid_argument);
    EXPECT_THROW(stage.setState("running"), std::invalid_argument);
    EXPECT_EQ(stage.getState(), "RUNNING");
}

TEST(AllowedStatesDecorator, EmptyPolicyRejectsAllRequestedChanges) {
    StageDefinition definition("review", "Review");
    AllowedStatesRuntimeDecorator stage(new StageInstance("case-1", &definition), {});
    EXPECT_THROW(stage.setState("IDLE"), std::invalid_argument);
    EXPECT_THROW(stage.setState("RUNNING"), std::invalid_argument);
    EXPECT_EQ(stage.getState(), "IDLE");
}

TEST(LoggingDecorator, ReportsActualBeforeAndAfterState) {
    StageDefinition definition("review", "Review");
    LoggingRuntimeDecorator stage(new StageInstance("case-1", &definition));
    DecoratorOutputCapture capture;
    stage.setState("RUNNING");
    EXPECT_EQ(stage.getState(), "RUNNING");
    EXPECT_EQ(capture.text(), "[STATE] review: IDLE -> RUNNING\n");
}

TEST(LoggingDecorator, DoesNotLogSuccessWhenWrappedValidationThrows) {
    StageDefinition definition("review", "Review");
    LoggingRuntimeDecorator stage(new AllowedStatesRuntimeDecorator(
        new StageInstance("case-1", &definition), {"IDLE"}));
    DecoratorOutputCapture capture;
    EXPECT_THROW(stage.setState("RUNNING"), std::invalid_argument);
    EXPECT_EQ(stage.getState(), "IDLE");
    EXPECT_TRUE(capture.text().empty());
}

TEST(LabelledDecorator, AddsIndentedContextAndPreservesOriginalOutput) {
    StageDefinition definition("review", "Review");
    LabelledRuntimeDecorator stage(new StageInstance("case-1", &definition), "Case 1 v2");
    DecoratorOutputCapture capture;
    stage.print(1);
    EXPECT_EQ(capture.text(), "  [Case 1 v2]\n    [Stage] Review (State: IDLE)\n");
    EXPECT_EQ(stage.getId(), "review");
    EXPECT_EQ(stage.getState(), "IDLE");
}

TEST(DecoratorComposition, ChecksSecurityThenValidationBeforeLoggingMutation) {
    StageDefinition definition("review", "Review");
    Participant reviewer("reviewer", "Reviewer", {"reviewer"});
    SecurityRuntimeDecorator stage(new AllowedStatesRuntimeDecorator(
        new LoggingRuntimeDecorator(new StageInstance("case-1", &definition)), {"IDLE", "RUNNING"}),
        "reviewer");
    RuntimeComponent& component = stage;  // Policies must also work via the common interface.
    DecoratorOutputCapture capture;
    EXPECT_THROW(component.setState("BANANA"), std::runtime_error);
    stage.setActor(&reviewer);
    EXPECT_THROW(component.setState("BANANA"), std::invalid_argument);
    EXPECT_EQ(component.getState(), "IDLE");
    EXPECT_TRUE(capture.text().empty());
    component.setState("RUNNING");
    EXPECT_EQ(component.getState(), "RUNNING");
    EXPECT_EQ(capture.text(), "[STATE] review: IDLE -> RUNNING\n");
}

TEST(DecoratorComposition, PreservesChildAccessAndVisitorTraversal) {
    StageDefinition definition("review", "Review"), childDefinition("child", "Child");
    LabelledRuntimeDecorator stage(new LoggingRuntimeDecorator(new StageInstance("case-1", &definition)), "Case 1");
    RuntimeComponent* child = new StageInstance("case-1", &childDefinition);
    stage.addChild(child);
    ASSERT_EQ(stage.getChildren().size(), 1u);
    EXPECT_EQ(stage.getChildren().front(), child);
    DecoratorStageVisitor visitor;
    stage.accept(visitor);
    const std::vector<std::string> expected = {"enter:review", "enter:child", "leave:child", "leave:review"};
    EXPECT_EQ(visitor.calls, expected);
}

TEST(PriorityDecorator, OrdersChildrenWithoutReorderingTheStoredTree) {
    StageDefinition definition("review", "Review"), lowDefinition("low", "Low"), highDefinition("high", "High");
    auto* original = new StageInstance("case-1", &definition);
    PriorityRuntimeDecorator stage(original, 0);
    auto* low = new PriorityRuntimeDecorator(new StageInstance("case-1", &lowDefinition), 1);
    auto* high = new PriorityRuntimeDecorator(new StageInstance("case-1", &highDefinition), 10);
    stage.addChild(new LabelledRuntimeDecorator(low, "Low priority"));
    stage.addChild(high);
    EXPECT_EQ(stage.getChildren().front()->getId(), "high");
    EXPECT_EQ(original->getChildren().front()->getId(), "low");
    low->setPriority(20);
    EXPECT_EQ(stage.getChildren().front()->getId(), "low");
    EXPECT_EQ(stage.getChildren().front()->getPriority(), 20);
}
