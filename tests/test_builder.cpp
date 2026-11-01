/**
 * @file test_builder.cpp
 * @brief GoogleTest unit tests for the Builder pattern.
 */
#include <gtest/gtest.h>
#include <stdexcept>

#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "ConcreteWorkflowBuilder.h"
#include "WorkflowDefinition.h"
#include "ActivityDefinition.h"
#include "StageDefinition.h"
#include "SubProcessDefinition.h"

namespace {

/** @brief Builds a minimal valid blueprint for reuse across tests. */
WorkflowBlueprint makeMinimalBlueprint() {
    WorkflowBlueprint bp;
    bp.id      = "test-workflow";
    bp.name    = "Test Workflow";
    bp.version = 1;
    bp.roles.push_back(RoleDefinition("user", "User", 0));
    bp.activities.push_back(ActivityDefinition("task1", "Task 1", "user"));
    return bp;
}

}

TEST(Builder, ProducesValidDefinition) {
    WorkflowBlueprint bp = makeMinimalBlueprint();
    WorkflowDirector director;
    ConcreteWorkflowBuilder builder;
    director.construct(builder, bp);
    WorkflowDefinition def = builder.build();

    EXPECT_EQ(def.getDefinitionId(), "test-workflow");
    EXPECT_EQ(def.getName(),         "Test Workflow");
    EXPECT_EQ(def.getVersion(),      1);
    EXPECT_EQ(def.getActivities().size(), 1);
    EXPECT_EQ(def.getRoles().size(),      1);
}

TEST(Builder, CopiesAllFlatLists) {
    WorkflowBlueprint bp;
    bp.id   = "w";
    bp.name = "W";
    bp.roles.push_back(RoleDefinition("a", "A", 0));
    bp.roles.push_back(RoleDefinition("b", "B", 1));
    bp.activities.push_back(ActivityDefinition("t1", "T1", "a"));
    bp.activities.push_back(ActivityDefinition("t2", "T2", "b"));
    bp.transitions.push_back(TransitionDefinition("t1", "t2", "always"));

    WorkflowDirector director;
    ConcreteWorkflowBuilder builder;
    director.construct(builder, bp);
    WorkflowDefinition def = builder.build();

    EXPECT_EQ(def.getRoles().size(),       2);
    EXPECT_EQ(def.getActivities().size(),  2);
    EXPECT_EQ(def.getTransitions().size(), 1);
    EXPECT_EQ(def.getTransitions()[0].getFrom(),      "t1");
    EXPECT_EQ(def.getTransitions()[0].getTo(),        "t2");
    EXPECT_EQ(def.getTransitions()[0].getCondition(), "always");
}


TEST(Builder, RejectsMissingId) {
    WorkflowBlueprint bp;
    bp.name = "No ID";
    bp.activities.push_back(ActivityDefinition("t", "T", "r"));

    WorkflowDirector director;
    ConcreteWorkflowBuilder builder;
    director.construct(builder, bp);

    EXPECT_THROW(builder.build(), std::runtime_error);
}

TEST(Builder, RejectsMissingName) {
    WorkflowBlueprint bp;
    bp.id = "no-name";
    bp.activities.push_back(ActivityDefinition("t", "T", "r"));

    WorkflowDirector director;
    ConcreteWorkflowBuilder builder;
    director.construct(builder, bp);

    EXPECT_THROW(builder.build(), std::runtime_error);
}

TEST(Builder, RejectsEmptyActivitiesAndNoTree) {
    WorkflowBlueprint bp;
    bp.id   = "empty";
    bp.name = "Empty";

    WorkflowDirector director;
    ConcreteWorkflowBuilder builder;
    director.construct(builder, bp);

    EXPECT_THROW(builder.build(), std::runtime_error);
}


TEST(Builder, BuildsCompositeTreeFromRootData) {
    WorkflowBlueprint bp;
    bp.id   = "tree-wf";
    bp.name = "Tree Workflow";

    WorkflowNodeData root;
    root.type = NodeType::STAGE;
    root.id   = "root";
    root.name = "Root";

    WorkflowNodeData a1;
    a1.type   = NodeType::ACTIVITY;
    a1.id     = "a1";
    a1.name   = "A1";
    a1.roleId = "user";
    root.children.push_back(a1);

    WorkflowNodeData inner;
    inner.type = NodeType::STAGE;
    inner.id   = "inner";
    inner.name = "Inner";

    WorkflowNodeData a2;
    a2.type   = NodeType::ACTIVITY;
    a2.id     = "a2";
    a2.name   = "A2";
    a2.roleId = "user";
    inner.children.push_back(a2);

    root.children.push_back(inner);

    bp.rootTreeData = root;
    bp.hasRootTree  = true;

    WorkflowDirector director;
    ConcreteWorkflowBuilder builder;
    director.construct(builder, bp);
    WorkflowDefinition def = builder.build();

    ASSERT_NE(def.getRoot(), nullptr);

    auto* rootStage = dynamic_cast<StageDefinition*>(def.getRoot());
    ASSERT_NE(rootStage, nullptr);
    EXPECT_EQ(rootStage->getId(), "root");
    EXPECT_EQ(rootStage->getChildren().size(), 2);

    auto* child1 = dynamic_cast<ActivityDefinition*>(rootStage->getChildren()[0]);
    ASSERT_NE(child1, nullptr);
    EXPECT_EQ(child1->getId(), "a1");

    auto* child2 = dynamic_cast<StageDefinition*>(rootStage->getChildren()[1]);
    ASSERT_NE(child2, nullptr);
    EXPECT_EQ(child2->getId(), "inner");
    EXPECT_EQ(child2->getChildren().size(), 1);
}

TEST(Builder, BuildsSubProcessNode) {
    WorkflowBlueprint bp;
    bp.id   = "sub-wf";
    bp.name = "Sub Workflow";

    WorkflowNodeData root;
    root.type = NodeType::STAGE;
    root.id   = "root";
    root.name = "Root";

    WorkflowNodeData sub;
    sub.type          = NodeType::SUBPROCESS;
    sub.id            = "call-child";
    sub.name          = "Call Child";
    sub.calledDefId   = "child-workflow";
    sub.calledVersion = 2;
    root.children.push_back(sub);

    bp.rootTreeData = root;
    bp.hasRootTree  = true;

    WorkflowDirector director;
    ConcreteWorkflowBuilder builder;
    director.construct(builder, bp);
    WorkflowDefinition def = builder.build();

    auto* stage = dynamic_cast<StageDefinition*>(def.getRoot());
    ASSERT_NE(stage, nullptr);
    ASSERT_EQ(stage->getChildren().size(), 1);

    auto* subDef = dynamic_cast<SubProcessDefinition*>(stage->getChildren()[0]);
    ASSERT_NE(subDef, nullptr);
    EXPECT_EQ(subDef->getCalledDefinitionId(), "child-workflow");
    EXPECT_EQ(subDef->getCalledVersion(),      2);
}


TEST(Builder, MutatingBlueprintAfterBuildDoesNotAffectDefinition) {
    WorkflowBlueprint bp = makeMinimalBlueprint();
    WorkflowDirector d; ConcreteWorkflowBuilder b;
    d.construct(b, bp);
    WorkflowDefinition def = b.build();
    int before = def.getActivities().size();

    bp.activities.push_back(ActivityDefinition("new", "New", "user"));
    EXPECT_EQ(def.getActivities().size(), before);   
}

TEST(Builder, AcceptsOnlyRootTreeWithNoFlatActivities) {
    WorkflowBlueprint bp;
    bp.id = "tree-only"; bp.name = "Tree Only";
    WorkflowNodeData root; root.type = NodeType::STAGE;
    root.id = "r"; root.name = "Root";
    bp.rootTreeData = root; bp.hasRootTree = true;

    WorkflowDirector d; ConcreteWorkflowBuilder b;
    d.construct(b, bp);
    WorkflowDefinition def = b.build();
    EXPECT_EQ(def.getActivities().size(), 0);
    EXPECT_NE(def.getRoot(), nullptr);
}

TEST(Builder, CopiesApprovalChains) {
    WorkflowBlueprint bp = makeMinimalBlueprint();
    ApprovalChainDefinition chain("c1");
    chain.addStep(ApprovalStepDefinition("manager", 3600, "escalate"));
    bp.approvalChains.push_back(chain);

    WorkflowDirector d; ConcreteWorkflowBuilder b;
    d.construct(b, bp);
    WorkflowDefinition def = b.build();
    EXPECT_EQ(def.getApprovalChains().size(), 1);
}