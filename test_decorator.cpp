// Each test follows the same steps: create a stage, wrap it, then check behavior.
// Decorators own their wrapped components. Definitions stay alive until the test ends.
#include <iostream>
#include <sstream>
#include "StageDefinition.h"
#include "StageInstance.h"
#include "Participant.h"
#include "SecurityRuntimeDecorator.h"
#include "AllowedStatesRuntimeDecorator.h"
#include "LoggingRuntimeDecorator.h"
#include "LabelledRuntimeDecorator.h"


namespace {
int passed = 0;
int failed = 0;

void check(bool condition, const std::string& message) {
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << message << '\n';
    if (condition) ++passed;
    else ++failed;
}

// Temporarily capture printed output so logging/labels can be checked automatically.
// Leaving the scope restores cout, even if an exception occurs.
class CaptureOutput {
    std::ostringstream output;
    std::streambuf* previous;
public:
    CaptureOutput() : previous(std::cout.rdbuf(output.rdbuf())) {}
    ~CaptureOutput() { std::cout.rdbuf(previous); }
    std::string text() const { return output.str(); }
};

void testSecurity() {
    std::cout << "\n1. SECURITY: only a reviewer may change the stage\n";
    StageDefinition definition("review", "Review stage");
    Participant reviewer("p1", "Alice", {"reviewer"});
    Participant visitor("p2", "Visitor", {"guest"});
    SecurityRuntimeDecorator stage(
        new StageInstance("case-1", &definition), "reviewer");

    bool denied = false;
    try { stage.setState("RUNNING"); }
    catch (const std::runtime_error&) { denied = true; }
    check(denied && stage.getState() == "IDLE", "No actor: update is rejected");

    stage.setActor(&visitor);
    denied = false;
    try { stage.setState("RUNNING"); }
    catch (const std::runtime_error&) { denied = true; }
    check(denied && !stage.canModify() && stage.getState() == "IDLE",
          "Wrong role: update is rejected and state stays IDLE");

    stage.setActor(&reviewer);
    stage.setState("RUNNING");
    check(stage.canModify() && stage.getState() == "RUNNING",
          "Correct role: update succeeds");

    stage.setActor(nullptr);
    denied = false;
    try { stage.setState("COMPLETED"); }
    catch (const std::runtime_error&) { denied = true; }
    check(denied && stage.getState() == "RUNNING", "Removing the actor revokes access");

    StageDefinition childDefinition("child", "Child stage");
    // This child must be deleted by the caller if insertion is refused.
    auto* child = new StageInstance("case-1", &childDefinition);
    denied = false;
    try { stage.addChild(child); }
    catch (const std::runtime_error&) { denied = true; }
    check(denied && stage.getChildren().empty(), "Unauthorized child insertion is rejected");
    if (denied) delete child;

    stage.setActor(&reviewer);
    stage.addChild(new StageInstance("case-1", &childDefinition));
    check(stage.getChildren().size() == 1, "Authorized child insertion succeeds");
}

void testAllowedStates() {
    std::cout << "\n2. ALLOWED STATES: reject unknown state names\n";
    StageDefinition definition("review", "Review stage");
    AllowedStatesRuntimeDecorator stage(
        new StageInstance("case-1", &definition), {"IDLE", "RUNNING", "COMPLETED"});

    bool rejected = false;
    try { stage.setState("BANANA"); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected && stage.getState() == "IDLE", "BANANA is rejected without changing state");

    stage.setState("RUNNING");
    check(stage.getState() == "RUNNING", "An allowed state is accepted");
    // This decorator validates names, not lifecycle transition order.
}

void testLogging() {
    std::cout << "\n3. LOGGING: print the old and new states\n";
    StageDefinition definition("review", "Review stage");
    LoggingRuntimeDecorator stage(new StageInstance("case-1", &definition));
    std::string log;
    {
        CaptureOutput capture;
        stage.setState("RUNNING");
        log = capture.text();
    }
    check(stage.getState() == "RUNNING", "Logging still forwards the update");
    check(log == "[STATE] review: IDLE -> RUNNING\n", "Log contains the ID and both states");
}

void testLabel() {
    std::cout << "\n4. LABEL: add a heading to printed output\n";
    StageDefinition definition("review", "Review stage");
    LabelledRuntimeDecorator stage(new StageInstance("case-1", &definition), "Campus incident");
    std::string printed;
    {
        CaptureOutput capture;
        stage.print();
        printed = capture.text();
    }
    check(printed == "[Campus incident]\n  [Stage] Review stage (State: IDLE)\n",
          "Label appears above the original stage output");
    check(stage.getId() == "review" && stage.getState() == "IDLE",
          "Adding a label preserves the component ID and state");
}

void testStackedDecorators() {
    std::cout << "\n5. STACKING: Security -> AllowedStates -> Logging -> Stage\n";
    StageDefinition definition("review", "Review stage");
    Participant reviewer("p1", "Alice", {"reviewer"});
    Participant visitor("p2", "Visitor", {"guest"});
    SecurityRuntimeDecorator stage(
        new AllowedStatesRuntimeDecorator(
            new LoggingRuntimeDecorator(new StageInstance("case-1", &definition)),
            {"IDLE", "RUNNING"}),
        "reviewer", &visitor);

    bool accessDenied = false;
    bool invalidState = false;
    std::string log;
    {
        CaptureOutput capture;
        try { stage.setState("RUNNING"); }
        catch (const std::runtime_error&) { accessDenied = true; }
        stage.setActor(&reviewer);
        try { stage.setState("BANANA"); }
        catch (const std::invalid_argument&) { invalidState = true; }
        log = capture.text();
    }
    check(accessDenied && invalidState && stage.getState() == "IDLE",
          "Security and validation both reject bad requests");
    check(log.empty(), "Rejected requests never reach Logging");
    {
        CaptureOutput capture;
        stage.setState("RUNNING");
        log = capture.text();
    }
    check(stage.getState() == "RUNNING" && log == "[STATE] review: IDLE -> RUNNING\n",
          "An authorized, valid request updates the stage and is logged");
}

void testNullComponent() {
    std::cout << "\n6. CONSTRUCTION: a decorator needs a component\n";
    bool rejected = false;
    try { RuntimeComponentDecorator stage(nullptr); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "The shared decorator base rejects nullptr");
}
} // namespace

int main() {
    try {
        testSecurity();
        testAllowedStates();
        testLogging();
        testLabel();
        testStackedDecorators();
        testNullComponent();
    } catch (const std::exception& error) {
        check(false, std::string("Unexpected exception: ") + error.what());
    }
    std::cout << "\nRESULT: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
