/**
 * @file test_composite.cpp
 * @brief GoogleTest unit tests for the Composite pattern.
 */
#include <gtest/gtest.h>

#include "WorkflowComponent.h"
#include "ActivityDefinition.h"
#include "StageDefinition.h"
#include "SubProcessDefinition.h"

// Leaf behaviour 

TEST(Composite, ActivityIsLeaf) {
    ActivityDefinition act("a", "A", "user");
    EXPECT_TRUE(act.getChildren().empty());
    EXPECT_EQ(act.getId(),             "a");
    EXPECT_EQ(act.getName(),           "A");
    EXPECT_EQ(act.getRequiredRoleId(), "user");
}

TEST(Composite, SubProcessIsLeaf) {
    SubProcessDefinition sp("sp", "child-wf", 3);
    EXPECT_TRUE(sp.getChildren().empty());
    EXPECT_EQ(sp.getId(),                    "sp");
    EXPECT_EQ(sp.getCalledDefinitionId(),    "child-wf");
    EXPECT_EQ(sp.getCalledVersion(),         3);
}

TEST(Composite, ActivityAddChildIsNoOp) {
    ActivityDefinition act("a", "A", "user");
    ActivityDefinition child("b", "B", "user");
    act.addChild(&child);   // base class prints "Cannot add child to a leaf"
    EXPECT_TRUE(act.getChildren().empty());
}

// Composite behaviour

TEST(Composite, StageCanAddChildren) {
    StageDefinition stage("s", "Stage");
    stage.addChild(new ActivityDefinition("a1", "A1", "user"));
    stage.addChild(new ActivityDefinition("a2", "A2", "user"));

    EXPECT_EQ(stage.getChildren().size(), 2);
    // stage owns the two activities — its destructor deletes them
}

TEST(Composite, StageCanNestStages) {
    StageDefinition outer("outer", "Outer");
    auto* inner = new StageDefinition("inner", "Inner");
    inner->addChild(new ActivityDefinition("a", "A", "user"));
    outer.addChild(inner);

    EXPECT_EQ(outer.getChildren().size(), 1);
    auto* retrievedInner = dynamic_cast<StageDefinition*>(outer.getChildren()[0]);
    ASSERT_NE(retrievedInner, nullptr);
    EXPECT_EQ(retrievedInner->getId(), "inner");
    EXPECT_EQ(retrievedInner->getChildren().size(), 1);
}

TEST(Composite, DestructorRecursivelyDeletesChildren) {
    // Relies on AddressSanitizer to detect leaks if destructor misbehaves
    auto* stage = new StageDefinition("s", "Stage");
    stage->addChild(new ActivityDefinition("a1", "A1", "user"));

    auto* inner = new StageDefinition("i", "Inner");
    inner->addChild(new ActivityDefinition("a2", "A2", "user"));
    stage->addChild(inner);

    delete stage;   // must delete a1, inner, and a2 recursively
    SUCCEED();
}

TEST(Composite, SubProcessStoresReference) {
    SubProcessDefinition sp("call", "child-def", 5);
    EXPECT_EQ(sp.getCalledDefinitionId(), "child-def");
    EXPECT_EQ(sp.getCalledVersion(),      5);
}

//Tree traversal

namespace {
int countLeaves(const WorkflowComponent* node) {
    if (!node) return 0;

    // A leaf is a node that is NOT a composite — check the concrete type,
    // not whether it has children. An empty stage is still a composite.
    if (dynamic_cast<const StageDefinition*>(node) != nullptr) {
        // Composite — recurse into children (an empty stage returns 0)
        int total = 0;
        for (const WorkflowComponent* child : node->getChildren()) {
            total += countLeaves(child);
        }
        return total;
    }

    // Anything else (ActivityDefinition, SubProcessDefinition) is a leaf
    return 1;
}
}

TEST(Composite, TreeTraversalCountsLeaves) {
    auto* root = new StageDefinition("root", "Root");
    root->addChild(new ActivityDefinition("a1", "A1", "user"));

    auto* inner = new StageDefinition("inner", "Inner");
    inner->addChild(new ActivityDefinition("a2", "A2", "user"));
    inner->addChild(new ActivityDefinition("a3", "A3", "user"));
    root->addChild(inner);

    EXPECT_EQ(countLeaves(root), 3);   // a1, a2, a3
    delete root;
}

TEST(Composite, IDsAndNamesPropagate) {
    StageDefinition s("my-id", "My Name");
    EXPECT_EQ(s.getId(),   "my-id");
    EXPECT_EQ(s.getName(), "My Name");
}

TEST(Composite, PreservesChildInsertionOrder) {
    StageDefinition s("s", "Stage");
    s.addChild(new ActivityDefinition("a", "A", "user"));
    s.addChild(new ActivityDefinition("b", "B", "user"));
    s.addChild(new ActivityDefinition("c", "C", "user"));

    auto children = s.getChildren();
    ASSERT_EQ(children.size(), 3);
    EXPECT_EQ(children[0]->getId(), "a");
    EXPECT_EQ(children[1]->getId(), "b");
    EXPECT_EQ(children[2]->getId(), "c");
}

TEST(Composite, DeepNestingTraversal) {
    // 5 levels deep
    auto* root = new StageDefinition("L0", "L0");
    auto* current = root;
    for (int i = 1; i <= 4; ++i) {
        auto* next = new StageDefinition("L" + std::to_string(i), "L");
        current->addChild(next);
        current = next;
    }
    current->addChild(new ActivityDefinition("leaf", "Leaf", "user"));

    EXPECT_EQ(countLeaves(root), 1);
    delete root;
}

TEST(Composite, EmptyStageTraversal) {
    StageDefinition s("empty", "Empty");
    EXPECT_EQ(countLeaves(&s), 0);
    EXPECT_TRUE(s.getChildren().empty());
}

TEST(Composite, MixedContentStage) {
    StageDefinition s("s", "Stage");
    s.addChild(new ActivityDefinition("a", "A", "user"));
    s.addChild(new SubProcessDefinition("sp", "child", 1));
    EXPECT_EQ(s.getChildren().size(), 2);
}

TEST(Composite, SubProcessIsLeafEvenWhenNested) {
    StageDefinition s("s", "Stage");
    auto* sp = new SubProcessDefinition("sp", "child", 1);
    s.addChild(sp);
    EXPECT_TRUE(sp->getChildren().empty());
}

TEST(Composite, PrintIncludesIds) {
    StageDefinition s("stage-id", "Stage Name");
    s.addChild(new ActivityDefinition("act-id", "Act Name", "user"));

    std::stringstream ss;
    auto* old = std::cout.rdbuf(ss.rdbuf());
    s.print();
    std::cout.rdbuf(old);

    std::string out = ss.str();
    EXPECT_NE(out.find("stage-id"), std::string::npos);
    EXPECT_NE(out.find("act-id"),   std::string::npos);
}