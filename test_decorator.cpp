#include "PriorityRuntimeDecorator.h"
#include "SecurityRuntimeDecorator.h"
#include "StageInstance.h"
#include "SubProcessInstance.h"
#include "WorkItem.h"
#include <sstream>

static int passed = 0, failed = 0;
static void check(const std::string& what, bool ok) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << '\n';
    if (ok) ++passed; else ++failed;
}
template <typename Exception, typename Operation>
static bool rejects(Operation operation) {
    try { operation(); } catch (const Exception&) { return true; }
    return false;
}
class TrackedLeaf : public SubProcessInstance {
    int& destructions;
public:
    TrackedLeaf(SubProcessDefinition* definition, int& count)
        : SubProcessInstance("instance", definition), destructions(count) {}
    ~TrackedLeaf() override { ++destructions; }
};

int main() {
    check("Base rejects null", rejects<std::invalid_argument>([] {
        RuntimeComponentDecorator node(nullptr);
    }));
    check("Priority rejects null", rejects<std::invalid_argument>([] {
        PriorityRuntimeDecorator node(nullptr, 1);
    }));
    check("Security rejects null", rejects<std::invalid_argument>([] {
        SecurityRuntimeDecorator node(nullptr, "reviewer");
    }));
    Participant reviewer("r", "Reviewer", {"reviewer"});
    Participant visitor("v", "Visitor", {"viewer"});
    SubProcessDefinition leaf("call", "approval", 1);
    ActivityDefinition activity("task", "Review task", "reviewer");
    StageDefinition stage("stage", "Review");
    check("Empty role rejected", rejects<std::invalid_argument>([&] {
        SecurityRuntimeDecorator node(new SubProcessInstance("i", &leaf), "");
    }));

    SecurityRuntimeDecorator secured(new WorkItem("i", &activity), "reviewer");
    RuntimeComponent& component = secured;
    check("Anonymous mutation denied", rejects<std::runtime_error>([&] {
        component.setState("RUNNING");
    }) && component.getState() == "IDLE");
    secured.setActor(&visitor);
    check("Wrong role denied", !secured.canModify() && rejects<std::runtime_error>([&] {
        component.setState("DONE");
    }) && component.getState() == "IDLE");
    secured.setActor(&reviewer);
    component.setState("RUNNING");
    check("Authorized WorkItem mutation", secured.canModify() && component.getState() == "RUNNING");
    check("Leaf interface preserved", component.getId() == "task" && component.getChildren().empty()
          && component.getPriority() == 0);
    secured.setActor(nullptr);
    check("Clearing actor revokes access", rejects<std::runtime_error>([&] {
        component.setState("DONE");
    }) && component.getState() == "RUNNING");

    int destructions = 0;
    {
        auto* rawStage = new StageInstance("i", &stage);
        auto* guard = new SecurityRuntimeDecorator(rawStage, "reviewer");
        std::unique_ptr<RuntimeComponent> root(new PriorityRuntimeDecorator(guard, 5));
        std::unique_ptr<RuntimeComponent> pending(new TrackedLeaf(&leaf, destructions));
        check("Denied insertion retains caller ownership", rejects<std::runtime_error>([&] {
            root->addChild(pending.get());
        }) && root->getChildren().empty() && destructions == 0);
        guard->setActor(&reviewer);
        RuntimeComponent* normal = pending.get();
        root->addChild(pending.get());
        pending.release();
        auto* first = new PriorityRuntimeDecorator(new TrackedLeaf(&leaf, destructions), 10);
        auto* second = new SecurityRuntimeDecorator(
            new PriorityRuntimeDecorator(new TrackedLeaf(&leaf, destructions), 10), "reviewer", &reviewer);
        root->addChild(first);
        root->addChild(second);
        auto children = root->getChildren();
        check("Highest priority first; ties retain insertion order",
              children.size() == 3 && children[0] == first && children[1] == second && children[2] == normal);
        check("Security forwards inner priority", second->getPriority() == 10);
        check("Stored composite order unchanged", rawStage->getChildren()[0] == normal);
        first->setPriority(-1);
        children = root->getChildren();
        check("Priority changes affect next query", children[0] == second && children[1] == normal && children[2] == first);
        root->setState("RUNNING");
        check("Priority outside security forwards authorized changes", root->getState() == "RUNNING" && root->getPriority() == 5);
        guard->setActor(&visitor);
        check("Priority outside security preserves denial", rejects<std::runtime_error>([&] {
            root->setState("DONE");
        }) && root->getState() == "RUNNING");
        second->setState("DONE");
        second->setActor(&visitor);
        check("Security outside priority preserves denial", rejects<std::runtime_error>([&] {
            second->setState("RUNNING");
        }) && second->getState() == "DONE");
        std::ostringstream output;
        auto* previous = std::cout.rdbuf(output.rdbuf());
        root->print(1);
        std::cout.rdbuf(previous);
        check("Print still delegates without added labels", output.str().find("  [Stage] Review (State: RUNNING)\n") == 0);
    }
    check("Stacked decorators destroy owned subtree exactly once", destructions == 3);
    std::cout << "=== RESULT: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
