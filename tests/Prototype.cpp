
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

#include "ActivityDefinition.h"
#include "StageDefinition.h"
#include "SubProcessDefinition.h"
#include "WorkflowDefinition.h"
#include "RoleDefinition.h"
#include "TransitionDefinition.h"
#include "ApprovalChainDefinition.h"

#include "InMemoryDefinitionRepository.h"
#include "WorkflowEngine.h"
#include "EventBus.h"
#include "HistoryManager.h"
#include "AuditObserver.h"
#include "RoleBasedAssignment.h"
#include "Participant.h"

// --------------------------------------------------
// Helper functions
// --------------------------------------------------

/**
 * Builds a sample workflow tree:
 *
 * Root stage
 *  |
 *  |-- Intake activity
 *  |
 *  |-- Review stage
 *  |     |
 *  |     |-- Check activity
 *  |
 *  |-- Payroll subprocess
 */
static StageDefinition* buildSampleTree() {
    StageDefinition* root =
        new StageDefinition("root", "Leave request");

    root->addChild(
        new ActivityDefinition("intake", "Intake", "clerk")
    );

    StageDefinition* review =
        new StageDefinition("review", "Review");

    review->addChild(
        new ActivityDefinition("check", "Check", "manager", "chain1")
    );

    root->addChild(review);

    root->addChild(
        new SubProcessDefinition("pay", "payroll", 3)
    );

    return root;
}

/**
 * Counts the total number of nodes in a workflow tree.
 */
static int countNodes(const WorkflowComponent* node) {
    int total = 1;

    for (const WorkflowComponent* child : node->getChildren()) {
        total += countNodes(child);
    }

    return total;
}

/**
 * Collects the memory addresses of every node in a tree.
 */
static void collect(
    const WorkflowComponent* node,
    std::vector<const WorkflowComponent*>& result
) {
    result.push_back(node);

    for (const WorkflowComponent* child : node->getChildren()) {
        collect(child, result);
    }
}

/**
 * Checks whether two trees share any node objects.
 */
static bool sharesNoNodes(
    const WorkflowComponent* first,
    const WorkflowComponent* second
) {
    std::vector<const WorkflowComponent*> firstNodes;
    std::vector<const WorkflowComponent*> secondNodes;

    collect(first, firstNodes);
    collect(second, secondNodes);

    for (const WorkflowComponent* a : firstNodes) {
        for (const WorkflowComponent* b : secondNodes) {
            if (a == b) {
                return false;
            }
        }
    }

    return true;
}

/**
 * Builds a complete sample workflow definition.
 */
static WorkflowDefinition* buildSampleDefinition() {
    WorkflowDefinition* definition =
        new WorkflowDefinition("leave", "Leave request", 1);

    definition->addRole(
        RoleDefinition("clerk", "Clerk", 1)
    );

    definition->addRole(
        RoleDefinition("manager", "Manager", 2)
    );

    definition->addActivity(
        ActivityDefinition("intake", "Intake", "clerk")
    );

    definition->addActivity(
        ActivityDefinition("check", "Check", "manager", "chain1")
    );

    definition->addTransition(
        TransitionDefinition("intake", "check", "always")
    );

    ApprovalChainDefinition chain("chain1");

    chain.addStep(
        ApprovalStepDefinition("manager", 60, "escalate")
    );

    definition->addApprovalChain(chain);

    definition->setRoot(buildSampleTree());

    return definition;
}

// --------------------------------------------------
// TEST 1: ActivityDefinition cloning
// --------------------------------------------------

TEST(PrototypeTest, ActivityDefinitionClonePreservesItsFields) {
    ActivityDefinition original(
        "a1", "Submit", "clerk", "chain9"
    );

    std::unique_ptr<ActivityDefinition> copy(original.clone());

    ASSERT_NE(copy.get(), nullptr);

    EXPECT_NE(copy.get(), &original);

    EXPECT_EQ(copy->getId(), "a1");
    EXPECT_EQ(copy->getName(), "Submit");
    EXPECT_EQ(copy->getRequiredRoleId(), "clerk");
    EXPECT_EQ(copy->getApprovalChainId(), "chain9");
}

// --------------------------------------------------
// TEST 2: SubProcessDefinition cloning
// --------------------------------------------------

TEST(PrototypeTest, SubProcessDefinitionClonePreservesItsFields) {
    SubProcessDefinition original("sp1", "payroll", 3);

    std::unique_ptr<SubProcessDefinition> copy(original.clone());

    ASSERT_NE(copy.get(), nullptr);

    EXPECT_NE(copy.get(), &original);

    EXPECT_EQ(copy->getId(), "sp1");
    EXPECT_EQ(copy->getCalledDefinitionId(), "payroll");
    EXPECT_EQ(copy->getCalledVersion(), 3);
}

// --------------------------------------------------
// TEST 3: StageDefinition deep copying
// --------------------------------------------------

TEST(PrototypeTest, StageDefinitionCloneCreatesIndependentDeepCopy) {
    std::unique_ptr<StageDefinition> original(buildSampleTree());

    std::unique_ptr<WorkflowComponent> copy(original->clone());

    ASSERT_NE(copy.get(), nullptr);

    EXPECT_NE(copy.get(), original.get());

    EXPECT_EQ(copy->getId(), "root");
    EXPECT_EQ(copy->getName(), "Leave request");

    EXPECT_EQ(countNodes(copy.get()), countNodes(original.get()));
    EXPECT_EQ(copy->getChildren().size(), 3u);

    // Check that the original child order was preserved.
    EXPECT_EQ(copy->getChildren()[0]->getId(), "intake");
    EXPECT_EQ(copy->getChildren()[1]->getId(), "review");
    EXPECT_EQ(copy->getChildren()[2]->getId(), "pay");

    // Check that the nested stage was also copied.
    EXPECT_EQ(
        copy->getChildren()[1]->getChildren().size(),
        1u
    );

    EXPECT_TRUE(sharesNoNodes(original.get(), copy.get()));

    // Check that the cloned child types are preserved.
    EXPECT_NE(
        dynamic_cast<ActivityDefinition*>(copy->getChildren()[0]),
        nullptr
    );

    EXPECT_NE(
        dynamic_cast<StageDefinition*>(copy->getChildren()[1]),
        nullptr
    );

    EXPECT_NE(
        dynamic_cast<SubProcessDefinition*>(copy->getChildren()[2]),
        nullptr
    );

    // Check fields on a deeply nested activity.
    ActivityDefinition* nestedCopy =
        dynamic_cast<ActivityDefinition*>(
            copy->getChildren()[1]->getChildren()[0]
        );

    ASSERT_NE(nestedCopy, nullptr);

    EXPECT_EQ(nestedCopy->getRequiredRoleId(), "manager");
    EXPECT_EQ(nestedCopy->getApprovalChainId(), "chain1");
}

// --------------------------------------------------
// TEST 4: Original and cloned tree independence
// --------------------------------------------------

TEST(PrototypeTest, ChangesToOneTreeDoNotAffectTheOther) {
    std::unique_ptr<StageDefinition> original(buildSampleTree());

    std::unique_ptr<StageDefinition> copy(original->clone());

    int originalNodeCount = countNodes(original.get());

    // Add two nodes to the cloned tree.
    copy->addChild(
        new ActivityDefinition("extra", "Extra step", "clerk")
    );

    copy->getChildren()[1]->addChild(
        new ActivityDefinition("extra2", "Extra nested", "clerk")
    );

    EXPECT_EQ(countNodes(original.get()), originalNodeCount);
    EXPECT_EQ(countNodes(copy.get()), originalNodeCount + 2);

    EXPECT_EQ(original->getChildren().size(), 3u);

    // Now modify the original tree.
    original->addChild(
        new ActivityDefinition("late", "Late step", "clerk")
    );

    // The cloned tree must remain unchanged.
    EXPECT_EQ(countNodes(copy.get()), originalNodeCount + 2);
    EXPECT_EQ(countNodes(original.get()), originalNodeCount + 1);
}

// --------------------------------------------------
// TEST 5: Safe destruction of cloned trees
// --------------------------------------------------

TEST(PrototypeTest, DestroyingOriginalDoesNotDestroyClone) {
    std::unique_ptr<StageDefinition> original(buildSampleTree());

    std::unique_ptr<StageDefinition> copy(original->clone());

    ASSERT_NE(copy.get(), nullptr);

    // Destroy the original tree completely.
    original.reset();

    // The cloned tree must still be usable.
    EXPECT_EQ(countNodes(copy.get()), 5);

    EXPECT_EQ(
        copy->getChildren()[1]->getChildren()[0]->getName(),
        "Check"
    );

    // The copy is automatically destroyed when unique_ptr goes out of scope.
}

// --------------------------------------------------
// TEST 6: WorkflowDefinition version cloning
// --------------------------------------------------

TEST(PrototypeTest, CloneWithVersionCreatesNewVersionAndCopiesDefinition) {
    std::unique_ptr<WorkflowDefinition> v1(
        buildSampleDefinition()
    );

    std::unique_ptr<WorkflowDefinition> v2(
        v1->cloneWithVersion(2)
    );

    ASSERT_NE(v2.get(), nullptr);

    EXPECT_NE(v2.get(), v1.get());

    EXPECT_EQ(v2->getVersion(), 2);
    EXPECT_EQ(v1->getVersion(), 1);

    EXPECT_NE(v1->getVersion(), v2->getVersion());

    EXPECT_EQ(v2->getDefinitionId(), "leave");
    EXPECT_EQ(v2->getName(), "Leave request");

    // Check that all definition collections were copied.
    EXPECT_EQ(v2->getRoles().size(), 2u);
    EXPECT_EQ(v2->getActivities().size(), 2u);
    EXPECT_EQ(v2->getTransitions().size(), 1u);

    // Check the approval chain.
    ASSERT_EQ(v2->getApprovalChains().size(), 1u);

    ASSERT_NE(
        v2->getApprovalChainById("chain1"),
        nullptr
    );

    EXPECT_EQ(
        v2->getApprovalChainById("chain1")->getSteps().size(),
        1u
    );

    // Check that the workflow tree was also copied.
    ASSERT_NE(v2->getRoot(), nullptr);

    EXPECT_NE(v2->getRoot(), v1->getRoot());

    EXPECT_EQ(
        countNodes(v2->getRoot()),
        countNodes(v1->getRoot())
    );

    EXPECT_TRUE(
        sharesNoNodes(v1->getRoot(), v2->getRoot())
    );

    // Modify every collection in the cloned definition.
    v2->getRoot()->addChild(
        new ActivityDefinition("new", "New step", "clerk")
    );

    v2->addRole(
        RoleDefinition("auditor", "Auditor", 3)
    );

    v2->addActivity(
        ActivityDefinition("new", "New step", "clerk")
    );

    v2->addTransition(
        TransitionDefinition("check", "new", "always")
    );

    v2->getApprovalChainById("chain1")->addStep(
        ApprovalStepDefinition("auditor", 30, "escalate")
    );

    // Verify that none of those changes affected the original.
    EXPECT_EQ(countNodes(v1->getRoot()), 5);

    EXPECT_EQ(v1->getRoles().size(), 2u);
    EXPECT_EQ(v1->getActivities().size(), 2u);
    EXPECT_EQ(v1->getTransitions().size(), 1u);

    EXPECT_EQ(
        v1->getApprovalChainById("chain1")->getSteps().size(),
        1u
    );
}

// --------------------------------------------------
// TEST 7: WorkflowDefinition edge cases
// --------------------------------------------------

TEST(PrototypeTest, CloneWithVersionHandlesEdgeCases) {
    std::unique_ptr<WorkflowDefinition> v1(
        buildSampleDefinition()
    );

    // Cloning with the same version should throw an exception.
    EXPECT_THROW(
        v1->cloneWithVersion(1),
        std::invalid_argument
    );

    // A definition without a tree should still be cloneable.
    WorkflowDefinition empty("blank", "No tree yet", 1);

    std::unique_ptr<WorkflowDefinition> emptyCopy(
        empty.cloneWithVersion(2)
    );

    ASSERT_NE(emptyCopy.get(), nullptr);

    EXPECT_EQ(emptyCopy->getVersion(), 2);
    EXPECT_EQ(emptyCopy->getRoot(), nullptr);

    // A cloned definition should itself be clonable.
    std::unique_ptr<WorkflowDefinition> v2(
        v1->cloneWithVersion(2)
    );

    std::unique_ptr<WorkflowDefinition> v3(
        v2->cloneWithVersion(3)
    );

    ASSERT_NE(v3.get(), nullptr);

    EXPECT_EQ(v3->getVersion(), 3);

    EXPECT_TRUE(
        sharesNoNodes(v2->getRoot(), v3->getRoot())
    );
}

// --------------------------------------------------
// TEST 8: Prototype integration with repository
//         and WorkflowEngine
// --------------------------------------------------

TEST(PrototypeIntegrationTest, ClonedDefinitionWorksWithRepositoryAndEngine) {
    InMemoryDefinitionRepository repository;
    EventBus eventBus;
    HistoryManager history;
    AuditObserver audit;

    WorkflowEngine engine(
        &repository,
        &eventBus,
        &history,
        &audit
    );

    Participant clerk(
        "p1", "Casey",
        std::vector<std::string>{"clerk"},
        1
    );

    Participant manager(
        "p2", "Morgan",
        std::vector<std::string>{"manager"},
        2
    );

    RoleBasedAssignment assignment;

    engine.setAssignment(
        &assignment,
        std::vector<Participant*>{&clerk, &manager}
    );

    // Create and save version 1.
    WorkflowDefinition* v1 =
        new WorkflowDefinition("leave", "Leave request", 1);

    StageDefinition* root =
        new StageDefinition("root", "Leave request");

    root->addChild(
        new ActivityDefinition("intake", "Intake", "clerk")
    );

    root->addChild(
        new ActivityDefinition("check", "Check", "manager")
    );

    v1->setRoot(root);

    // The repository owns v1 after saving it.
    repository.save(v1);

    // Clone version 1 into version 2.
    WorkflowDefinition* v2 = v1->cloneWithVersion(2);

    // Add another activity to the cloned version.
    v2->getRoot()->addChild(
        new ActivityDefinition("sign", "Sign off", "manager")
    );

    // The repository owns v2 after saving it.
    repository.save(v2);

    // Verify repository lookup.
    EXPECT_EQ(repository.load("leave", 1), v1);
    EXPECT_EQ(repository.load("leave", 2), v2);
    EXPECT_EQ(repository.loadLatest("leave"), v2);

    // Start workflow instances using both versions.
    std::string id1 = engine.startWorkflow("leave", 1);
    std::string id2 = engine.startWorkflow("leave", 2);

    EXPECT_FALSE(id1.empty());
    EXPECT_FALSE(id2.empty());

    ASSERT_NE(engine.getInstance(id1), nullptr);
    ASSERT_NE(engine.getInstance(id2), nullptr);

    // Version 1 contains two activities.
    EXPECT_EQ(
        engine.getInstance(id1)->getWorkItems().size(),
        2u
    );

    // Version 2 contains the additional activity.
    EXPECT_EQ(
        engine.getInstance(id2)->getWorkItems().size(),
        3u
    );

    // Each instance remains associated with its original version.
    EXPECT_EQ(
        engine.getInstance(id1)->getDefinitionVersion(),
        1
    );

    EXPECT_EQ(
        engine.getInstance(id2)->getDefinitionVersion(),
        2
    );
}
