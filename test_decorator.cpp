// Each test follows the same steps: create a stage, wrap it, then check behavior.
// Decorators own their wrapped components. Definitions stay alive until the test ends.
#include <iostream>
#include <sstream>
#include "StageDefinition.h"
#include "StageInstance.h"
#include "Participant.h"
#include "SecurityRuntimeDecorator.h"
<<<<<<< HEAD
#include "PasswordRuntimeDecorator.h"
#include "DependencyRuntimeDecorator.h"
=======
>>>>>>> 66ddb2ab7c5301769e2aac6233190d45b7b28d31
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

<<<<<<< HEAD
void testPasswordAndNames() {
    StageDefinition definition("protected", "Protected stage");
    Participant admin("admin", "Admin", {"administrator"});
    Participant otherAdmin("other", "Other admin", {"administrator"});
    Participant guest("guest", "Guest", {"guest"});
    auto* password = new PasswordRuntimeDecorator(new StageInstance("case", &definition), "demo-secret");
    SecurityRuntimeDecorator stage(password, "administrator", &admin);
    RuntimeComponent& component = stage;
    check(component.getId() == "protected" && component.getName() ==
          "Protected stage [requires access key] [requires role: administrator]",
          "Polymorphic names compose requirements while IDs remain stable");
    password->setActor(&admin);
    bool ran = false, denied = false;
    try { component.execute([&]() { ran = true; return true; }); }
    catch (const std::runtime_error&) { denied = true; }
    check(denied && !ran, "Locked decorator refuses commands before executing them");
    check(!password->enterKey("wrong"), "Wrong key is refused");
    denied = false;
    try { component.setState("RUNNING"); }
    catch (const std::runtime_error&) { denied = true; }
    check(denied && component.getState() == "IDLE", "Wrong key leaves state unchanged");
    check(password->enterKey("demo-secret"), "Correct key unlocks the current participant");
    check(component.execute([&]() { ran = true; return true; }) && ran,
          "Both requirements allow command execution");
    component.setState("RUNNING");
    check(component.getState() == "RUNNING", "Both requirements allow state changes");
    stage.setActor(&guest);
    denied = false;
    try { component.setState("COMPLETED"); }
    catch (const std::runtime_error&) { denied = true; }
    check(denied && component.getState() == "RUNNING", "A key does not grant the required role");
    password->setActor(&otherAdmin);
    check(!password->canModify(), "Changing participants revokes the key grant");
    password->enterKey("demo-secret");
    check(!password->enterKey("wrong") && !password->canModify(), "Wrong key revokes an earlier grant");
    password->enterKey("demo-secret");
    password->lock();
    denied = false;
    std::unique_ptr<RuntimeComponent> child(new StageInstance("case", &definition));
    try { password->addChild(child.get()); }
    catch (const std::runtime_error&) { denied = true; }
    check(denied && password->getChildren().empty(), "Locked insertion leaves ownership with the caller");
    check(!admin.hasRole("demo-secret") && component.getName().find("demo-secret") == std::string::npos,
          "Keys are neither roles nor part of the display name");

    auto* security = new SecurityRuntimeDecorator(new StageInstance("case", &definition), "administrator", &guest);
    PasswordRuntimeDecorator reversed(security, "another-secret");
    reversed.setActor(&guest);
    reversed.enterKey("another-secret");
    denied = false;
    try { reversed.execute([]() { return true; }); }
    catch (const std::runtime_error&) { denied = true; }
    check(denied, "Checks compose with the password decorator outside the role decorator");
    reversed.setActor(&admin);
    check(!reversed.canModify(), "Actor changes propagate through the full decorator stack");
    reversed.enterKey("another-secret");
    check(reversed.execute([]() { return true; }), "Reversed stack succeeds when both checks pass");

    bool invalid = false;
    try { PasswordRuntimeDecorator empty(new StageInstance("case", &definition), ""); }
    catch (const std::invalid_argument&) { invalid = true; }
    check(invalid, "Empty access keys are rejected");
}

void testLiveDependencies() {
    StageDefinition definition("restore", "Restore service");
    Participant admin("admin", "Admin", {"administrator"});
    Participant guest("guest", "Guest", {"guest"});
    bool prerequisiteComplete = false;
    auto* password = new PasswordRuntimeDecorator(new StageInstance("case", &definition),
                                                   "hidden-key", "recovery key");
    DependencyRuntimeDecorator component(new SecurityRuntimeDecorator(password, "administrator"),
        "complete Backup", [&]() { return prerequisiteComplete ? "" : "complete Backup first; its state is Created"; });
    component.setActor(&guest);
    bool ran = false;
    std::string reason;
    try { component.execute([&]() { ran = true; return true; }); }
    catch (const std::runtime_error& error) { reason = error.what(); }
    check(!ran && reason.find("Backup") != std::string::npos,
          "Dependency failure names the actual prerequisite before running a command");
    prerequisiteComplete = true;
    reason.clear();
    try { component.execute([&]() { ran = true; return true; }); }
    catch (const std::runtime_error& error) { reason = error.what(); }
    check(!ran && reason.find("roles: guest") != std::string::npos &&
          reason.find("required role: administrator") != std::string::npos,
          "A wrong role is explained by the security decorator");
    component.setActor(&admin);
    reason.clear();
    try { component.execute([&]() { ran = true; return true; }); }
    catch (const std::runtime_error& error) { reason = error.what(); }
    check(!ran && reason.find("missing access key for Admin: recovery key") != std::string::npos,
          "The password decorator identifies the participant and missing named key");
    password->enterKey("hidden-key");
    check(component.execute([&]() { ran = true; return true; }) && ran,
          "Live dependency, role and password checks all permit the command");
    component.setState("RUNNING");
    prerequisiteComplete = false;
    bool denied = false;
    try { component.setState("COMPLETED"); }
    catch (const std::runtime_error&) { denied = true; }
    check(denied && component.getState() == "RUNNING",
          "Changing a prerequisite revokes access without rebuilding the decorators");
    check(component.getName().find("dependencies: complete Backup") != std::string::npos &&
          component.getName().find("hidden-key") == std::string::npos,
          "Names show the dependency and key requirement without revealing the secret");
    prerequisiteComplete = true;
    component.setActor(nullptr);
    reason.clear();
    try { component.execute([]() { return true; }); }
    catch (const std::runtime_error& error) { reason = error.what(); }
    check(reason.find("no participant assigned or selected") != std::string::npos,
          "Missing participants receive an explicit explanation");
}

=======
>>>>>>> 66ddb2ab7c5301769e2aac6233190d45b7b28d31
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
<<<<<<< HEAD
        testPasswordAndNames();
        testLiveDependencies();
=======
>>>>>>> 66ddb2ab7c5301769e2aac6233190d45b7b28d31
    } catch (const std::exception& error) {
        check(false, std::string("Unexpected exception: ") + error.what());
    }
    std::cout << "\nRESULT: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
