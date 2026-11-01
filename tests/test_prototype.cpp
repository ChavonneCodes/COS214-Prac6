/**
 * @file test_prototype.cpp
 * @brief GoogleTest unit tests for the Prototype pattern on the workflow definition tree.
 *
 * Covers deep copies (clone), the new version number of a cloned definition,
 * isolation between the original and the copy, and invalid input.
 */
#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "ActivityDefinition.h"
#include "ApprovalChainDefinition.h"
#include "InMemoryDefinitionRepository.h"
#include "RoleDefinition.h"
#include "StageDefinition.h"
#include "SubProcessDefinition.h"
#include "TransitionDefinition.h"
#include "WorkflowDefinition.h"

namespace {

/** root stage -> [ intake, review stage -> [ check ], call to payroll v3 ]  (5 nodes) */
StageDefinition* buildSampleTree() {
    StageDefinition* root = new StageDefinition("root", "Leave request");
    root->addChild(new ActivityDefinition("intake", "Intake", "clerk"));
    StageDefinition* review = new StageDefinition("review", "Review");
    review->addChild(new ActivityDefinition("check", "Check", "manager", "chain1"));
    root->addChild(review);
    root->addChild(new SubProcessDefinition("pay", "payroll", 3));
    return root;
}

int countNodes(const WorkflowComponent* node) {
    int total = 1;
    std::vector<WorkflowComponent*> children = node->getChildren();
    for (size_t i = 0; i < children.size(); ++i) total += countNodes(children[i]);
    return total;
}

void collect(const WorkflowComponent* node, std::vector<const WorkflowComponent*>& out) {
    out.push_back(node);
    std::vector<WorkflowComponent*> children = node->getChildren();
    for (size_t i = 0; i < children.size(); ++i) collect(children[i], out);
}

/** True when the two trees have no node (pointer) in common. */
bool sharesNoNodes(const WorkflowComponent* a, const WorkflowComponent* b) {
    std::vector<const WorkflowComponent*> x, y;
    collect(a, x);
    collect(b, y);
    for (size_t i = 0; i < x.size(); ++i)
        for (size_t j = 0; j < y.size(); ++j)
            if (x[i] == y[j]) return false;
    return true;
}

/** "leave" v1 with 2 roles, 2 activities, 1 transition, 1 approval chain and the sample tree. */
WorkflowDefinition* buildSampleDefinition() {
    WorkflowDefinition* def = new WorkflowDefinition("leave", "Leave request", 1);
    def->addRole(RoleDefinition("clerk", "Clerk", 1));
    def->addRole(RoleDefinition("manager", "Manager", 2));
    def->addActivity(ActivityDefinition("intake", "Intake", "clerk"));
    def->addActivity(ActivityDefinition("check", "Check", "manager", "chain1"));
    def->addTransition(TransitionDefinition("intake", "check", "always"));
    ApprovalChainDefinition chain("chain1");
    chain.addStep(ApprovalStepDefinition("manager", 60, "escalate"));
    def->addApprovalChain(chain);
    def->setRoot(buildSampleTree());
    return def;
}

}  // namespace

//  Leaf nodes 

TEST(PrototypeLeaf, ActivityCloneIsAnEqualButSeparateObject) {
    ActivityDefinition original("a1", "Submit", "clerk", "chain9");
    std::unique_ptr<ActivityDefinition> copy(original.clone());
    EXPECT_NE(copy.get(), &original);
    EXPECT_EQ(copy->getId(), "a1");
    EXPECT_EQ(copy->getName(), "Submit");
    EXPECT_EQ(copy->getRequiredRoleId(), "clerk");
    EXPECT_EQ(copy->getApprovalChainId(), "chain9");
}

TEST(PrototypeLeaf, SubProcessCloneStillCallsTheSameDefinitionAndVersion) {
    SubProcessDefinition original("sp1", "payroll", 3);
    std::unique_ptr<SubProcessDefinition> copy(original.clone());
    EXPECT_NE(copy.get(), &original);
    EXPECT_EQ(copy->getId(), "sp1");
    EXPECT_EQ(copy->getCalledDefinitionId(), "payroll");
    EXPECT_EQ(copy->getCalledVersion(), 3);
}

//  Composite nodes 

class StageCloneTest : public ::testing::Test {
protected:
    std::unique_ptr<StageDefinition> original{buildSampleTree()};
};

TEST_F(StageCloneTest, CloneCopiesTheWholeSubtree) {
    std::unique_ptr<WorkflowComponent> copy(original->clone());  // through the base class

    EXPECT_NE(copy.get(), original.get());
    EXPECT_EQ(copy->getId(), "root");
    EXPECT_EQ(copy->getName(), "Leave request");
    EXPECT_EQ(countNodes(copy.get()), countNodes(original.get()));

    std::vector<WorkflowComponent*> kids = copy->getChildren();
    ASSERT_EQ(kids.size(), 3u);
    EXPECT_EQ(kids[0]->getId(), "intake");
    EXPECT_EQ(kids[1]->getId(), "review");
    EXPECT_EQ(kids[2]->getId(), "pay");
    EXPECT_NE(dynamic_cast<ActivityDefinition*>(kids[0]), nullptr);
    EXPECT_NE(dynamic_cast<StageDefinition*>(kids[1]), nullptr);
    EXPECT_NE(dynamic_cast<SubProcessDefinition*>(kids[2]), nullptr);
    ASSERT_EQ(kids[1]->getChildren().size(), 1u);
    EXPECT_EQ(kids[1]->getChildren()[0]->getId(), "check");
}

TEST_F(StageCloneTest, CloneSharesNoNodeWithTheOriginal) {
    std::unique_ptr<StageDefinition> copy(original->clone());
    EXPECT_TRUE(sharesNoNodes(original.get(), copy.get()));
}

TEST_F(StageCloneTest, EditingTheCopyDoesNotChangeTheOriginal) {
    std::unique_ptr<StageDefinition> copy(original->clone());
    const int before = countNodes(original.get());

    copy->addChild(new ActivityDefinition("extra", "Extra", "clerk"));
    copy->getChildren()[1]->addChild(new ActivityDefinition("extra2", "Nested", "clerk"));

    EXPECT_EQ(countNodes(original.get()), before);
    EXPECT_EQ(countNodes(copy.get()), before + 2);
}

TEST_F(StageCloneTest, EditingTheOriginalDoesNotChangeTheCopy) {
    std::unique_ptr<StageDefinition> copy(original->clone());
    const int before = countNodes(copy.get());

    original->addChild(new ActivityDefinition("late", "Late", "clerk"));

    EXPECT_EQ(countNodes(copy.get()), before);
}

TEST_F(StageCloneTest, DeletingTheOriginalLeavesTheCopyUsable) {
    std::unique_ptr<StageDefinition> copy(original->clone());
    original.reset();
    EXPECT_EQ(countNodes(copy.get()), 5);
    EXPECT_EQ(copy->getChildren()[1]->getChildren()[0]->getName(), "Check");
}

//  Whole definitions 

class DefinitionCloneTest : public ::testing::Test {
protected:
    std::unique_ptr<WorkflowDefinition> v1{buildSampleDefinition()};
};

TEST_F(DefinitionCloneTest, CopyHasANewVersionAndTheOriginalKeepsItsOwn) {
    std::unique_ptr<WorkflowDefinition> v2(v1->cloneWithVersion(2));
    EXPECT_NE(v2.get(), v1.get());
    EXPECT_EQ(v2->getVersion(), 2);
    EXPECT_EQ(v1->getVersion(), 1);
}

TEST_F(DefinitionCloneTest, CopyKeepsTheIdNameAndAllTheLists) {
    std::unique_ptr<WorkflowDefinition> v2(v1->cloneWithVersion(2));
    EXPECT_EQ(v2->getDefinitionId(), "leave");
    EXPECT_EQ(v2->getName(), "Leave request");
    EXPECT_EQ(v2->getRoles().size(), 2u);
    EXPECT_EQ(v2->getActivities().size(), 2u);
    EXPECT_EQ(v2->getTransitions().size(), 1u);
    ASSERT_NE(v2->getApprovalChainById("chain1"), nullptr);
    EXPECT_EQ(v2->getApprovalChainById("chain1")->getSteps().size(), 1u);
}

TEST_F(DefinitionCloneTest, CopyHasItsOwnCopyOfTheTree) {
    std::unique_ptr<WorkflowDefinition> v2(v1->cloneWithVersion(2));
    ASSERT_NE(v2->getRoot(), nullptr);
    EXPECT_NE(v2->getRoot(), v1->getRoot());
    EXPECT_EQ(countNodes(v2->getRoot()), countNodes(v1->getRoot()));
    EXPECT_TRUE(sharesNoNodes(v1->getRoot(), v2->getRoot()));
}

TEST_F(DefinitionCloneTest, EditingEveryPartOfTheCopyLeavesTheOriginalUntouched) {
    std::unique_ptr<WorkflowDefinition> v2(v1->cloneWithVersion(2));

    v2->getRoot()->addChild(new ActivityDefinition("new", "New", "clerk"));
    v2->addRole(RoleDefinition("auditor", "Auditor", 3));
    v2->addActivity(ActivityDefinition("new", "New", "clerk"));
    v2->addTransition(TransitionDefinition("check", "new", "always"));
    v2->getApprovalChainById("chain1")->addStep(ApprovalStepDefinition("auditor", 30, "escalate"));

    EXPECT_EQ(countNodes(v1->getRoot()), 5);
    EXPECT_EQ(v1->getRoles().size(), 2u);
    EXPECT_EQ(v1->getActivities().size(), 2u);
    EXPECT_EQ(v1->getTransitions().size(), 1u);
    EXPECT_EQ(v1->getApprovalChainById("chain1")->getSteps().size(), 1u);
}

TEST_F(DefinitionCloneTest, ACopyCanItselfBeCloned) {
    std::unique_ptr<WorkflowDefinition> v2(v1->cloneWithVersion(2));
    std::unique_ptr<WorkflowDefinition> v3(v2->cloneWithVersion(3));
    EXPECT_EQ(v3->getVersion(), 3);
    EXPECT_TRUE(sharesNoNodes(v2->getRoot(), v3->getRoot()));
}

//  Invalid input 

TEST_F(DefinitionCloneTest, TheSameVersionNumberIsRefused) {
    EXPECT_THROW(v1->cloneWithVersion(1), std::invalid_argument);
}

TEST(DefinitionCloneEdgeCases, ADefinitionWithNoTreeYetCanStillBeCloned) {
    WorkflowDefinition empty("blank", "No tree yet", 1);
    std::unique_ptr<WorkflowDefinition> copy(empty.cloneWithVersion(2));
    EXPECT_EQ(copy->getRoot(), nullptr);
    EXPECT_EQ(copy->getVersion(), 2);
}

//  Works with the repository 

TEST(DefinitionCloneRepository, ACopyCanBeStoredNextToTheOriginal) {
    InMemoryDefinitionRepository repository;
    WorkflowDefinition* v1 = buildSampleDefinition();
    repository.save(v1);  // the repository takes ownership
    WorkflowDefinition* v2 = v1->cloneWithVersion(2);
    repository.save(v2);

    EXPECT_EQ(repository.load("leave", 1), v1);
    EXPECT_EQ(repository.load("leave", 2), v2);
    EXPECT_EQ(repository.loadLatest("leave"), v2);
}
