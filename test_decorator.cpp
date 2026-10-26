#include "AllowedStatesRuntimeDecorator.h"
#include "LabelledRuntimeDecorator.h"
#include "LoggingRuntimeDecorator.h"
#include "StageInstance.h"
#include "SubProcessInstance.h"
#include <sstream>

static int passed = 0, failed = 0;

static void check(const std::string& what, bool ok) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << '\n';
    if (ok) ++passed; else ++failed;
}

// Restore cout even when a decorated operation throws.
class OutputCapture {
    std::ostringstream output;
    std::streambuf* previous;
public:
    OutputCapture() : previous(std::cout.rdbuf(output.rdbuf())) {}
    ~OutputCapture() { std::cout.rdbuf(previous); }
    std::string str() const { return output.str(); }
    OutputCapture(const OutputCapture&) = delete;
    OutputCapture& operator=(const OutputCapture&) = delete;
};

template <typename Operation>
static bool rejects(Operation operation) {
    try {
        operation();
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

// Count destruction while retaining the real runtime leaf behavior.
class TrackedLeaf : public SubProcessInstance {
    int& destructions;
public:
    TrackedLeaf(SubProcessDefinition* definition, int& destructions)
        : SubProcessInstance("instance", definition), destructions(destructions) {}
    ~TrackedLeaf() override { ++destructions; }
};

int main() {
    check("Base decorator rejects nullptr", rejects([] {
        RuntimeComponentDecorator decorator(nullptr);
    }));
    check("Logging decorator rejects nullptr", rejects([] {
        LoggingRuntimeDecorator decorator(nullptr);
    }));
    check("Label decorator rejects nullptr", rejects([] {
        LabelledRuntimeDecorator decorator(nullptr, "Review");
    }));
    check("Allowed states decorator rejects nullptr", rejects([] {
        AllowedStatesRuntimeDecorator decorator(nullptr, {"IDLE"});
    }));

    SubProcessDefinition leafDefinition("call", "approval", 1);
    RuntimeComponentDecorator base(new SubProcessInstance("instance", &leafDefinition));
    RuntimeComponent& component = base;
    check("Delegates leaf identifier", component.getId() == "call");
    check("Delegates initial state", component.getState() == "IDLE");
    component.setState("RUNNING");
    check("Delegates state changes", component.getState() == "RUNNING");
    check("Wrapped leaf has no children", component.getChildren().empty());
    std::string printed;
    {
        OutputCapture capture;
        component.print(2);
        printed = capture.str();
    }
    check("Preserves print indentation",
          printed == "    [SubProcessInstance] Calls approval (State: RUNNING)\n");

    AllowedStatesRuntimeDecorator allowed(
        new SubProcessInstance("instance", &leafDefinition), {"IDLE", "DONE"});
    allowed.setState("DONE");
    check("Accepts an allowed state", allowed.getState() == "DONE");
    check("Rejects unknown states", rejects([&] { allowed.setState("UNKNOWN"); }));
    check("Rejected change preserves state", allowed.getState() == "DONE");
    check("State names are case-sensitive", rejects([&] { allowed.setState("done"); }));
    allowed.setState("IDLE");
    check("Does not impose transition order", allowed.getState() == "IDLE");
    AllowedStatesRuntimeDecorator empty(
        new SubProcessInstance("instance", &leafDefinition), {});
    check("Empty allowed set rejects even the current state",
          rejects([&] { empty.setState("IDLE"); }) && empty.getState() == "IDLE");

    // Logging outside validation must not log a rejected change.
    LoggingRuntimeDecorator logging(new AllowedStatesRuntimeDecorator(
        new SubProcessInstance("instance", &leafDefinition), {"IDLE", "DONE"}));
    std::string log;
    {
        OutputCapture capture;
        logging.setState("DONE");
        log = capture.str();
    }
    check("Logs identifier and old/new states", log == "[STATE] call: IDLE -> DONE\n");
    bool rejected = false;
    {
        OutputCapture capture;
        rejected = rejects([&] { logging.setState("INVALID"); });
        log = capture.str();
    }
    check("Nested validation propagates rejection", rejected && logging.getState() == "DONE");
    check("Rejected changes produce no log", log.empty());
    {
        OutputCapture capture;
        logging.setState("DONE");
        log = capture.str();
    }
    check("Normally returning unchanged state is logged", log == "[STATE] call: DONE -> DONE\n");

    StageDefinition stageDefinition("review", "Review");
    int destructions = 0;
    {
        // Exercise all decorators together through the RuntimeComponent interface.
        std::unique_ptr<RuntimeComponent> decorated(new LabelledRuntimeDecorator(
            new LoggingRuntimeDecorator(new AllowedStatesRuntimeDecorator(
                new StageInstance("instance", &stageDefinition), {"IDLE", "DONE"})),
            "Approval"));
        RuntimeComponent* child = new TrackedLeaf(&leafDefinition, destructions);
        decorated->addChild(child);
        const auto children = decorated->getChildren();
        check("Nested decorators preserve composite children",
              children.size() == 1 && children[0] == child);
        {
            OutputCapture capture;
            decorated->setState("DONE");
            log = capture.str();
        }
        check("Stacked decorators update and log composite state",
              decorated->getId() == "review" && decorated->getState() == "DONE"
              && log == "[STATE] review: IDLE -> DONE\n");
        {
            OutputCapture capture;
            decorated->print(1);
            printed = capture.str();
        }
        check("Label indents the entire composite subtree",
              printed == "  [Approval]\n"
                         "    [Stage] Review (State: DONE)\n"
                         "      [SubProcessInstance] Calls approval (State: IDLE)\n");
        check("Child remains alive while wrapper owns the stage", destructions == 0);
    }
    check("Deleting the wrapper destroys the owned subtree exactly once", destructions == 1);

    std::cout << "=== RESULT: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
