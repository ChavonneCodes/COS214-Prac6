/**
 * @file test_prototype.cpp
 * @brief Tests for the Prototype pattern on the workflow definition tree.
 *
 * Checks that WorkflowComponent::clone() (ActivityDefinition, StageDefinition,
 * SubProcessDefinition) and WorkflowDefinition::cloneWithVersion() make deep,
 * independent copies, that the copy gets a different version, and that a
 * cloned definition still works with the repository and the engine.
 *
 * Build and run:  make test_prototype && ./test_prototype
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

static int passed = 0, failed = 0;
static void check(const std::string& what, bool ok) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << std::endl;
    if (ok) ++passed; else ++failed;
}

/** Builds: Root stage -> [ Intake activity, Review stage -> [ Check activity ], call to "payroll" v3 ]. */
static StageDefinition* buildSampleTree() {
    StageDefinition* root = new StageDefinition("root", "Leave request");
    root->addChild(new ActivityDefinition("intake", "Intake", "clerk"));
    StageDefinition* review = new StageDefinition("review", "Review");
    review->addChild(new ActivityDefinition("check", "Check", "manager", "chain1"));
    root->addChild(review);
    root->addChild(new SubProcessDefinition("pay", "payroll", 3));
    return root;
}

/** Counts every node in a tree (the node itself plus all descendants). */
static int countNodes(const WorkflowComponent* node) {
    int total = 1;
    for (const WorkflowComponent* child : node->getChildren()) total += countNodes(child);
    return total;
}

/** Collects the address of every node so two trees can be compared for sharing. */
static void collect(const WorkflowComponent* node, std::vector<const WorkflowComponent*>& out) {
    out.push_back(node);
    for (const WorkflowComponent* child : node->getChildren()) collect(child, out);
}

static bool sharesNoNodes(const WorkflowComponent* a, const WorkflowComponent* b) {
    std::vector<const WorkflowComponent*> x, y;
    collect(a, x);
    collect(b, y);
    for (const WorkflowComponent* p : x)
        for (const WorkflowComponent* q : y)
            if (p == q) return false;
    return true;
}

static void testActivityClone() {
    std::cout << "\n-- ActivityDefinition (leaf) --\n";
    ActivityDefinition original("a1", "Submit", "clerk", "chain9");
    std::unique_ptr<ActivityDefinition> copy(original.clone());
    check("clone is a different object", copy.get() != &original);
    check("same id", copy->getId() == "a1");
    check("same name", copy->getName() == "Submit");
    check("same required role", copy->getRequiredRoleId() == "clerk");
    check("same approval chain", copy->getApprovalChainId() == "chain9");
}

static void testSubProcessClone() {
    std::cout << "\n-- SubProcessDefinition (leaf) --\n";
    SubProcessDefinition original("sp1", "payroll", 3);
    std::unique_ptr<SubProcessDefinition> copy(original.clone());
    check("clone is a different object", copy.get() != &original);
    check("same id", copy->getId() == "sp1");
    check("still calls the same definition", copy->getCalledDefinitionId() == "payroll");
    check("still calls the same version (not re-versioned)", copy->getCalledVersion() == 3);
}

static void testStageDeepCopy() {
    std::cout << "\n-- StageDefinition (composite) deep copy --\n";
    std::unique_ptr<StageDefinition> original(buildSampleTree());
    std::unique_ptr<WorkflowComponent> copy(original->clone()); // through the base class

    check("copy is a different object", copy.get() != original.get());
    check("same id and name", copy->getId() == "root" && copy->getName() == "Leave request");
    check("same number of nodes", countNodes(copy.get()) == countNodes(original.get()));
    check("copy has 3 direct children", copy->getChildren().size() == 3);
    check("child order preserved",
          copy->getChildren()[0]->getId() == "intake" &&
          copy->getChildren()[1]->getId() == "review" &&
          copy->getChildren()[2]->getId() == "pay");
    check("nested stage was copied too", copy->getChildren()[1]->getChildren().size() == 1);
    check("no node is shared with the original", sharesNoNodes(original.get(), copy.get()));
    check("child types preserved (activity)",
          dynamic_cast<ActivityDefinition*>(copy->getChildren()[0]) != nullptr);
    check("child types preserved (stage)",
          dynamic_cast<StageDefinition*>(copy->getChildren()[1]) != nullptr);
    check("child types preserved (subprocess)",
          dynamic_cast<SubProcessDefinition*>(copy->getChildren()[2]) != nullptr);
    ActivityDefinition* nestedCopy = dynamic_cast<ActivityDefinition*>(copy->getChildren()[1]->getChildren()[0]);
    check("deep leaf keeps its fields", nestedCopy && nestedCopy->getRequiredRoleId() == "manager" &&
                                         nestedCopy->getApprovalChainId() == "chain1");
}

static void testCopyIsIndependent() {
    std::cout << "\n-- Editing the copy does not change the original (and the reverse) --\n";
    std::unique_ptr<StageDefinition> original(buildSampleTree());
    std::unique_ptr<StageDefinition> copy(original->clone());
    int before = countNodes(original.get());

    copy->addChild(new ActivityDefinition("extra", "Extra step", "clerk"));
    copy->getChildren()[1]->addChild(new ActivityDefinition("extra2", "Extra nested", "clerk"));
    check("original unchanged after editing the copy", countNodes(original.get()) == before);
    check("copy grew by two nodes", countNodes(copy.get()) == before + 2);
    check("original top level still has 3 children", original->getChildren().size() == 3);

    original->addChild(new ActivityDefinition("late", "Late step", "clerk"));
    check("copy unchanged after editing the original", countNodes(copy.get()) == before + 2);
}

static void testDestructionIsSafe() {
    std::cout << "\n-- Destroying one tree leaves the other intact --\n";
    std::unique_ptr<StageDefinition> original(buildSampleTree());
    StageDefinition* copy = original->clone();
    original.reset(); // frees the original tree completely
    check("copy still readable after the original is deleted",
          countNodes(copy) == 5 && copy->getChildren()[1]->getChildren()[0]->getName() == "Check");
    delete copy;      // valgrind / ASan would flag a double free here
    check("both trees deleted without a crash", true);
}

static WorkflowDefinition* buildSampleDefinition() {
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

static void testDefinitionClone() {
    std::cout << "\n-- WorkflowDefinition::cloneWithVersion --\n";
    std::unique_ptr<WorkflowDefinition> v1(buildSampleDefinition());
    std::unique_ptr<WorkflowDefinition> v2(v1->cloneWithVersion(2));

    check("copy is a different object", v2.get() != v1.get());
    check("copy has the new version", v2->getVersion() == 2);
    check("original keeps its version", v1->getVersion() == 1);
    check("versions differ", v1->getVersion() != v2->getVersion());
    check("same definition id", v2->getDefinitionId() == "leave");
    check("same name", v2->getName() == "Leave request");
    check("roles copied", v2->getRoles().size() == 2);
    check("activities copied", v2->getActivities().size() == 2);
    check("transitions copied", v2->getTransitions().size() == 1);
    check("approval chain copied", v2->getApprovalChains().size() == 1 &&
                                   v2->getApprovalChainById("chain1") != nullptr &&
                                   v2->getApprovalChainById("chain1")->getSteps().size() == 1);
    check("tree was copied", v2->getRoot() != nullptr && v2->getRoot() != v1->getRoot() &&
                             countNodes(v2->getRoot()) == countNodes(v1->getRoot()));
    check("tree shares no nodes with the original", sharesNoNodes(v1->getRoot(), v2->getRoot()));

    // Edit the copy in every way the class allows
    v2->getRoot()->addChild(new ActivityDefinition("new", "New step", "clerk"));
    v2->addRole(RoleDefinition("auditor", "Auditor", 3));
    v2->addActivity(ActivityDefinition("new", "New step", "clerk"));
    v2->addTransition(TransitionDefinition("check", "new", "always"));
    v2->getApprovalChainById("chain1")->addStep(ApprovalStepDefinition("auditor", 30, "escalate"));
    check("original tree untouched", countNodes(v1->getRoot()) == 5);
    check("original roles untouched", v1->getRoles().size() == 2);
    check("original activities untouched", v1->getActivities().size() == 2);
    check("original transitions untouched", v1->getTransitions().size() == 1);
    check("original approval chain untouched", v1->getApprovalChainById("chain1")->getSteps().size() == 1);
}

static void testDefinitionCloneEdgeCases() {
    std::cout << "\n-- Edge cases --\n";
    std::unique_ptr<WorkflowDefinition> v1(buildSampleDefinition());

    bool threw = false;
    try { delete v1->cloneWithVersion(1); } catch (const std::invalid_argument&) { threw = true; }
    check("same version number is rejected", threw);

    WorkflowDefinition empty("blank", "No tree yet", 1);
    std::unique_ptr<WorkflowDefinition> emptyCopy(empty.cloneWithVersion(2));
    check("definition without a tree can be cloned", emptyCopy->getRoot() == nullptr && emptyCopy->getVersion() == 2);

    std::unique_ptr<WorkflowDefinition> v2(v1->cloneWithVersion(2));
    std::unique_ptr<WorkflowDefinition> v3(v2->cloneWithVersion(3));
    check("a copy can itself be cloned", v3->getVersion() == 3 && sharesNoNodes(v2->getRoot(), v3->getRoot()));
}

static void testWorksWithRepositoryAndEngine() {
    std::cout << "\n-- Works with the repository and the engine --\n";
    InMemoryDefinitionRepository repository;
    EventBus eventBus;
    HistoryManager history;
    AuditObserver audit;
    WorkflowEngine engine(&repository, &eventBus, &history, &audit);

    Participant clerk("p1", "Casey", std::vector<std::string>{"clerk"}, 1);
    Participant manager("p2", "Morgan", std::vector<std::string>{"manager"}, 2);
    RoleBasedAssignment assignment;
    engine.setAssignment(&assignment, std::vector<Participant*>{&clerk, &manager});

    // A tree with only activities (no sub-process call), so the engine can run it as it is
    WorkflowDefinition* v1 = new WorkflowDefinition("leave", "Leave request", 1);
    StageDefinition* root = new StageDefinition("root", "Leave request");
    root->addChild(new ActivityDefinition("intake", "Intake", "clerk"));
    root->addChild(new ActivityDefinition("check", "Check", "manager"));
    v1->setRoot(root);
    repository.save(v1);                              // the repository owns it from here

    WorkflowDefinition* v2 = v1->cloneWithVersion(2); // clone the saved definition...
    v2->getRoot()->addChild(new ActivityDefinition("sign", "Sign off", "manager")); // ...and edit the copy
    repository.save(v2);

    check("repository finds version 1", repository.load("leave", 1) == v1);
    check("repository finds version 2", repository.load("leave", 2) == v2);
    check("loadLatest returns the new version", repository.loadLatest("leave") == v2);

    std::string id1 = engine.startWorkflow("leave", 1);
    std::string id2 = engine.startWorkflow("leave", 2);
    check("engine starts the original", !id1.empty());
    check("engine starts the cloned version", !id2.empty());
    check("original runs 2 work items", engine.getInstance(id1)->getWorkItems().size() == 2);
    check("clone runs 3 work items (its extra step)", engine.getInstance(id2)->getWorkItems().size() == 3);
    check("instance is pinned to the version it was started from",
          engine.getInstance(id1)->getDefinitionVersion() == 1 &&
          engine.getInstance(id2)->getDefinitionVersion() == 2);
}

int main() {
    std::cout << "===== Prototype pattern tests =====" << std::endl;
    testActivityClone();
    testSubProcessClone();
    testStageDeepCopy();
    testCopyIsIndependent();
    testDestructionIsSafe();
    testDefinitionClone();
    testDefinitionCloneEdgeCases();
    testWorksWithRepositoryAndEngine();

    std::cout << "\n" << passed << " passed, " << failed << " failed" << std::endl;
    return failed == 0 ? 0 : 1;
}
