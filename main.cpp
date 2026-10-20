// test_main.cpp - tests State (WorkItemState) and Command (WorkflowAction) behaviour.
// Does not modify any existing source file.
// Only <iostream>, <string> and <vector> are included.
// Note: invalid transitions print "Invalid State Transition" from your state classes.
//
// Build (from the project folder; excludes your own main.cpp):
//   g++ -g -std=c++11 -Wall -Wextra $(ls *.cpp | grep -v '^main.cpp$') -o TividyTest
//   ./TividyTest

#include <iostream>
#include <string>
#include <vector>

#include "ActivityDefinition.h"
#include "WorkItem.h"
#include "Participant.h"
#include "WorkflowAction.h"
#include "AssignCommand.h"
#include "StartCommand.h"
#include "CompleteCommand.h"
#include "RejectCommand.h"
#include "CancelCommand.h"
#include "EscalateCommand.h"

// ---------------------------------------------------------------- helpers
static int g_passed = 0;
static int g_failed = 0;

static void check(bool cond, const std::string& label) {
    if (cond) { ++g_passed; std::cout << "  [PASS] " << label << "\n"; }
    else      { ++g_failed; std::cout << "  [FAIL] " << label << "\n"; }
}

static void section(const std::string& t) {
    std::cout << "\n==== " << t << " ====\n";
}

static std::string stateOf(WorkItem& w) { return w.getCurrentState()->getName(); }

// Actions that exist on WorkItem
enum Action { ASSIGN, START, COMPLETE, REJECT, CANCEL, ESCALATE, FAIL, MAKE_AVAILABLE, ACTION_COUNT };
static const char* actionName[ACTION_COUNT] = {
    "assign", "start", "complete", "reject", "cancel", "escalated", "fail", "makeAvailable"
};

static bool doAction(WorkItem& w, Action a, Participant* p) {
    switch (a) {
        case ASSIGN:         return w.assign(p);
        case START:          return w.start();
        case COMPLETE:       return w.complete();
        case REJECT:         return w.reject();
        case CANCEL:         return w.cancel();
        case ESCALATE:       return w.escalated();
        case FAIL:           return w.fail();
        case MAKE_AVAILABLE: return w.makeAvailable();
        default:             return false;
    }
}

// Drive a fresh WorkItem into the named state using only legal transitions.
static void driveTo(WorkItem& w, const std::string& target, Participant* p) {
    if (target == "Created") return;
    if (target == "Cancelled")   { w.cancel(); return; }
    w.makeAvailable();
    if (target == "Available")   return;
    if (target == "Escalated")   { w.escalated(); return; }
    w.assign(p);
    if (target == "Assigned")    return;
    if (target == "Rejected")    { w.reject(); return; }
    w.start();
    if (target == "In_Progress") return;
    if (target == "Failed")      { w.fail(); return; }
    if (target == "Completed")   { w.complete(); return; }
}

// ---------------------------------------------------------------- tests
// Expected result of every action from every state. "" = invalid transition.
struct Row { const char* from; const char* to[ACTION_COUNT]; };
//                          assign        start         complete      reject      cancel       escalated    fail      makeAvailable
static const Row MATRIX[] = {
    {"Created",     {"",           "",            "",           "",         "Cancelled", "",          "",       "Available"}},
    {"Available",   {"Assigned",   "",            "",           "",         "Cancelled", "Escalated", "",       ""}},
    {"Assigned",    {"",           "In_Progress", "",           "Rejected", "Cancelled", "Escalated", "",       ""}},
    {"In_Progress", {"",           "",            "Completed",  "",         "Cancelled", "Escalated", "Failed", ""}},
    {"Escalated",   {"Assigned",   "",            "",           "",         "Cancelled", "",          "",       ""}},
    {"Rejected",    {"",           "",            "",           "",         "Cancelled", "",          "",       "Available"}},
    {"Failed",      {"",           "",            "",           "",         "Cancelled", "",          "",       "Available"}},
    {"Completed",   {"",           "",            "",           "",         "",          "",          "",       ""}},
    {"Cancelled",   {"",           "",            "",           "",         "",          "",          "",       ""}},
};

static void testInitialState(ActivityDefinition* def) {
    section("STATE: initial state");
    WorkItem w("wi-init", def);
    check(stateOf(w) == "Created", "new WorkItem starts in Created");
    check(w.getCurrentState() != nullptr, "getCurrentState() is non-null");
    check(w.getId() == "act1", "getId() returns the activity id");
}

static void testTransitionMatrix(ActivityDefinition* def, Participant* p) {
    section("STATE: full transition matrix (every state x every action)");
    const size_t rows = sizeof(MATRIX) / sizeof(MATRIX[0]);
    for (size_t r = 0; r < rows; ++r) {
        const Row& row = MATRIX[r];
        for (int a = 0; a < ACTION_COUNT; ++a) {
            WorkItem w("wi-matrix", def);
            driveTo(w, row.from, p);
            if (stateOf(w) != row.from) {
                check(false, std::string("could not drive item to ") + row.from);
                continue;
            }
            std::string expected = row.to[a];
            bool result;
            { result = doAction(w, static_cast<Action>(a), p); }

            std::string label = std::string(row.from) + " + " + actionName[a];
            if (expected.empty()) {
                check(!result && stateOf(w) == row.from,
                      label + " -> rejected, stays in " + row.from);
            } else {
                check(result && stateOf(w) == expected,
                      label + " -> " + expected);
            }
        }
    }
}

static void testInvalidTransitionMessage(ActivityDefinition* def) {
    section("STATE: invalid transition");
    WorkItem w("wi-msg", def);
    std::cout << "  (expect one 'Invalid State Transition' line below)\n";
    bool r = w.start();
    check(!r, "Created.start() returns false");
}

static void testAssignGuards(ActivityDefinition* def, Participant* p) {
    section("STATE: assign() guards");
    {
        WorkItem w("wi-null", def);
        driveTo(w, "Available", p);
        bool r; { r = w.assign(nullptr); }
        check(!r, "Available.assign(nullptr) returns false");
        check(stateOf(w) == "Available", "state unchanged after assign(nullptr)");
    }
    {
        WorkItem w("wi-esc-null", def);
        driveTo(w, "Escalated", p);
        bool r; { r = w.assign(nullptr); }
        check(!r, "Escalated.assign(nullptr) returns false");
        check(stateOf(w) == "Escalated", "state unchanged after assign(nullptr)");
    }
    {
        WorkItem w("wi-setp", def);
        w.setParticipant(nullptr);   // must be ignored, must not crash
        check(true, "setParticipant(nullptr) is ignored safely");
    }
}

static void testChangeState(ActivityDefinition* def) {
    section("STATE: WorkItem::changeState()");
    WorkItem w("wi-change", def);
    WorkItemState* before = w.getCurrentState();
    w.changeState(nullptr);
    check(w.getCurrentState() == before, "changeState(nullptr) is ignored");
    check(stateOf(w) == "Created", "state still Created after changeState(nullptr)");
}

static void testHappyPathStates(ActivityDefinition* def, Participant* p) {
    section("STATE: full happy path Created -> Completed");
    WorkItem w("wi-happy", def);
    bool a, b, c, d;
    std::string s1, s2, s3, s4;
    {
        a = w.makeAvailable(); s1 = stateOf(w);
        b = w.assign(p);       s2 = stateOf(w);
        c = w.start();         s3 = stateOf(w);
        d = w.complete();      s4 = stateOf(w);
    }
    check(a && s1 == "Available",   "Created -> Available");
    check(b && s2 == "Assigned",    "Available -> Assigned");
    check(c && s3 == "In_Progress", "Assigned -> In_Progress");
    check(d && s4 == "Completed",   "In_Progress -> Completed");
}

static void testRecoveryPaths(ActivityDefinition* def, Participant* p) {
    section("STATE: recovery loops");
    {
        WorkItem w("wi-rej", def);
        driveTo(w, "Rejected", p);
        bool r, r2;
        { r = w.makeAvailable(); r2 = w.assign(p); }
        check(r && r2 && stateOf(w) == "Assigned", "Rejected -> Available -> Assigned (re-offer)");
    }
    {
        WorkItem w("wi-fail", def);
        driveTo(w, "Failed", p);
        bool r;
        { r = w.makeAvailable(); }
        check(r && stateOf(w) == "Available", "Failed -> Available (retry)");
    }
    {
        WorkItem w("wi-esc", def);
        driveTo(w, "In_Progress", p);
        bool e, a, s;
        { e = w.escalated(); a = w.assign(p); s = w.start(); }
        check(e && a && s && stateOf(w) == "In_Progress",
              "In_Progress -> Escalated -> Assigned -> In_Progress");
    }
}

// ----------------------------------------------------------------- commands
static void testCommandsHappyPath(ActivityDefinition* def, Participant* p) {
    section("COMMAND: execute() drives the receiver's state");
    WorkItem w("wi-cmd", def);
    { w.makeAvailable(); }

    AssignCommand   assign(&w, p);
    StartCommand    start(&w);
    CompleteCommand complete(&w);

    // Use the base-class interface, as a Command invoker would.
    WorkflowAction* seq[] = { &assign, &start, &complete };
    const char* expectState[] = { "Assigned", "In_Progress", "Completed" };
    const char* names[] = { "AssignCommand", "StartCommand", "CompleteCommand" };
    for (int i = 0; i < 3; ++i) {
        bool r = seq[i]->execute();
        check(r && stateOf(w) == expectState[i],
              std::string(names[i]) + ".execute() -> " + expectState[i]);
    }
}

static void testCommandsReject(ActivityDefinition* def, Participant* p) {
    section("COMMAND: Reject / Escalate / Cancel");
    {
        WorkItem w("wi-c-rej", def);
        driveTo(w, "Assigned", p);
        RejectCommand c(&w);
        bool r = c.execute();
        check(r && stateOf(w) == "Rejected", "RejectCommand: Assigned -> Rejected");
    }
    {
        WorkItem w("wi-c-esc", def);
        driveTo(w, "In_Progress", p);
        std::string role = "manager";
        EscalateCommand c(&w, role);
        bool r = c.execute();
        check(r && stateOf(w) == "Escalated", "EscalateCommand: In_Progress -> Escalated");
    }
    {
        WorkItem w("wi-c-can", def);
        driveTo(w, "Assigned", p);
        CancelCommand c(&w);
        bool r = c.execute();
        check(r && stateOf(w) == "Cancelled", "CancelCommand: Assigned -> Cancelled");
    }
}

static void testCommandsInvalid(ActivityDefinition* def, Participant* p) {
    section("COMMAND: invalid execution is refused by the state");
    {
        WorkItem w("wi-bad1", def);                 // Created
        StartCommand c(&w);
        bool r; { r = c.execute(); }
        check(!r && stateOf(w) == "Created", "StartCommand on Created -> false, state unchanged");
    }
    {
        WorkItem w("wi-bad2", def);                 // Created
        CompleteCommand c(&w);
        bool r; { r = c.execute(); }
        check(!r && stateOf(w) == "Created", "CompleteCommand on Created -> false, state unchanged");
    }
    {
        WorkItem w("wi-bad3", def);
        driveTo(w, "Completed", p);
        CancelCommand c(&w);
        bool r; { r = c.execute(); }
        check(!r && stateOf(w) == "Completed", "CancelCommand on Completed -> false (terminal state)");
    }
    {
        WorkItem w("wi-bad4", def);
        driveTo(w, "Cancelled", p);
        StartCommand c(&w);
        bool r; { r = c.execute(); }
        check(!r && stateOf(w) == "Cancelled", "StartCommand on Cancelled -> false (terminal state)");
    }
    {
        WorkItem w("wi-bad5", def);
        driveTo(w, "Available", p);
        AssignCommand c(&w, nullptr);
        bool r; { r = c.execute(); }
        check(!r && stateOf(w) == "Available", "AssignCommand with null participant -> false");
    }
    {
        WorkItem w("wi-bad6", def);
        driveTo(w, "In_Progress", p);
        AssignCommand c(&w, p);
        bool r; { r = c.execute(); }
        check(!r && stateOf(w) == "In_Progress", "AssignCommand on In_Progress -> false");
    }
}

static void testCommandDescriptions(ActivityDefinition* def, Participant* p) {
    section("COMMAND: getDescription()");
    WorkItem w("wi-desc", def);
    std::string role = "manager";
    AssignCommand   a(&w, p);
    StartCommand    s(&w);
    CompleteCommand c(&w);
    RejectCommand   r(&w);
    CancelCommand   x(&w);
    EscalateCommand e(&w, role);

    // All descriptions should mention the WorkItem id.
    WorkflowAction* all[] = { &a, &s, &c, &r, &x, &e };
    const char* names[]   = { "Assign", "Start", "Complete", "Reject", "Cancel", "Escalate" };
    for (int i = 0; i < 6; ++i) {
        std::string d = all[i]->getDescription();
        check(d.find("act1") != std::string::npos,
              std::string(names[i]) + " description contains the item id: \"" + d + "\"");
    }
    check(e.getDescription().find("manager") != std::string::npos,
          "Escalate description contains the target role id");
    check(a.getDescription().find("Assigned")  != std::string::npos, "Assign description says Assigned");
    check(s.getDescription().find("started")   != std::string::npos, "Start description says started");
    check(r.getDescription().find("Rejected")  != std::string::npos, "Reject description says Rejected");
    check(x.getDescription().find("Cancelled") != std::string::npos, "Cancel description says Cancelled");
}

static void testCommandUndo(ActivityDefinition* def, Participant* p) {
    section("COMMAND: undo()");
    WorkItem w("wi-undo", def);
    { w.makeAvailable(); }
    AssignCommand a(&w, p);
    a.execute();
    std::cout << "  (expect 'Command undone' below)\n";
    a.undo();
    std::cout << "  [INFO] state after undo is '" << stateOf(w)
              << "' (undo() currently only prints; it does not revert the state)\n";
}

static void testCommandPolymorphicDelete(ActivityDefinition* def, Participant* p) {
    section("COMMAND: polymorphic ownership / virtual destructor");
    WorkItem w("wi-poly", def);
    { w.makeAvailable(); }
    std::vector<WorkflowAction*> queue;
    queue.push_back(new AssignCommand(&w, p));
    queue.push_back(new StartCommand(&w));
    queue.push_back(new CompleteCommand(&w));
    bool allOk = true;
    for (size_t i = 0; i < queue.size(); ++i) allOk = queue[i]->execute() && allOk;
    check(allOk && stateOf(w) == "Completed", "queued commands run in order -> Completed");
    for (size_t i = 0; i < queue.size(); ++i) delete queue[i];   // valgrind will confirm no leaks
    check(true, "deleting commands through WorkflowAction* works");
}

static void testObservation(ActivityDefinition* def, Participant* p) {
    section("INFO: WorkItem::getState() vs current State object");
    WorkItem w("wi-obs", def);
    { w.makeAvailable(); w.assign(p); }
    std::cout << "  [INFO] getCurrentState()->getName() = " << stateOf(w)
              << ", getState() = \"" << w.getState() << "\"\n"
              << "         (the two are separate; the state classes never call setState())\n";
}

// ----------------------------------------------------------------- main
int main() {
    std::cout << "TIVIDY: STATE + COMMAND TESTS\n";

    ActivityDefinition* def = new ActivityDefinition("act1", "Review", "role1");
    Participant* p = new Participant();

    testInitialState(def);
    testTransitionMatrix(def, p);
    testInvalidTransitionMessage(def);
    testAssignGuards(def, p);
    testChangeState(def);
    testHappyPathStates(def, p);
    testRecoveryPaths(def, p);

    testCommandsHappyPath(def, p);
    testCommandsReject(def, p);
    testCommandsInvalid(def, p);
    testCommandDescriptions(def, p);
    testCommandUndo(def, p);
    testCommandPolymorphicDelete(def, p);
    testObservation(def, p);

    delete p;
    delete def;

    std::cout << "\n==== SUMMARY ====\n"
              << "Passed: " << g_passed << "\nFailed: " << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
/*#include <iostream>
#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "ConcreteWorkflowBuilder.h"
#include "InstanceFactory.h"
#include "WorkflowDefinition.h"
#include "WorkItem.h"

int main() {
    std::cout << " TIVIDY: BUILDER + COMPOSITE + INSTANCE FACTORY INTEGRATION TEST\n";

    WorkflowDirector director;
    WorkflowBlueprint blueprint;
    blueprint.id = "project-approval";
    blueprint.name = "Tech Company Project Approval";
    blueprint.version = 1;

    std::cout << "--- PHASE 1: USER PROVIDES RAW DATA---\n";
    blueprint.roles.push_back(RoleDefinition("proposer", "Proposer", 0));
    blueprint.roles.push_back(RoleDefinition("team_lead", "Team Lead", 1));

    // The user constructs a raw data tree
    WorkflowNodeData rootData;
    rootData.type = NodeType::STAGE;
    rootData.id = "root";
    rootData.name = "Main Process";

    // Leaf 1: Submit Activity
    WorkflowNodeData submitData;
    submitData.type = NodeType::ACTIVITY;
    submitData.id = "submit";
    submitData.name = "Submit Proposal";
    submitData.roleId = "proposer";
    rootData.children.push_back(submitData);

    // Branch 1: Review Stage
    WorkflowNodeData reviewData;
    reviewData.type = NodeType::STAGE;
    reviewData.id = "review";
    reviewData.name = "Review Phase";
    
    WorkflowNodeData techReviewData;
    techReviewData.type = NodeType::ACTIVITY;
    techReviewData.id = "techReview";
    techReviewData.name = "Technical Review";
    techReviewData.roleId = "team_lead";
    reviewData.children.push_back(techReviewData);
    
    rootData.children.push_back(reviewData);

    // Branch 2: SubProcess
    WorkflowNodeData paymentData;
    paymentData.type = NodeType::SUBPROCESS;
    paymentData.id = "payment";
    paymentData.calledDefId = "payment-process";
    paymentData.calledVersion = 1;
    rootData.children.push_back(paymentData);

    // Give the raw tree to the blueprint
    blueprint.rootTreeData = rootData;
    blueprint.hasRootTree = true;
    std::cout << "User blueprint populated with raw data nodes.\n\n";


    std::cout << "--- PHASE 2: BUILDER PATTERN (Director & Concrete Builder) ---\n";
    std::cout << "Passing blueprint to Director and Concrete Builder...\n";
    ConcreteWorkflowBuilder builder;
    director.construct(builder, blueprint);
    
    // The Builder recursively creates the actual C++ objects here
    WorkflowDefinition def = builder.build(); 
    std::cout << "Successfully built WorkflowDefinition: " << def.getName() << "\n\n";


    std::cout << "--- PHASE 3: DEFINITION COMPOSITE TREE (The Static Blueprint) ---\n";
    std::cout << "This tree represents every possible path (immutable, read-only).\n";
    if (def.getRoot() != nullptr) {
        def.getRoot()->print();
    }
    std::cout << "\n";


    std::cout << "--- PHASE 4: INSTANTIATION (Creating the Runtime Tree) ---\n";
    std::string instanceId = "Claim-C-1001";
    std::cout << "Creating a running instance for ID: " << instanceId << "...\n";
    std::cout << "The InstanceFactory traverses the Definition Tree and generates\n";
    std::cout << "a Runtime Tree with mutable WorkItems and StageInstances.\n\n";

    RuntimeComponent* runtimeRoot = InstanceFactory::instantiateTree(instanceId, def.getRoot());
    
    std::cout << "Live Runtime Tree (Notice all states start as IDLE):\n";
    runtimeRoot->print();
    std::cout << "\n";


    std::cout << "--- PHASE 5: RUNTIME EXECUTION (Simulating Work) ---\n";
    std::cout << "Simulating a user completing the 'Submit Proposal' activity...\n";
    
    // Traverse the runtime tree and change the state of the 'submit' item
    auto children = runtimeRoot->getChildren();
    if (!children.empty()) {
        // Find the 'submit' activity specifically
        for (auto* child : children) {
            if (child->getId() == "submit") {
                child->setState("SUCCESS");
                std::cout << "-> State of '" << child->getId() << "' changed to SUCCESS.\n";
                break;
            }
        }
    }

    std::cout << "\nUpdated Runtime Tree:\n";
    runtimeRoot->print();

    std::cout << "\n=================================================================\n";
    std::cout << " TEST COMPLETE.\n";
    std::cout << " This proves the Definition Tree is read-only, while the Runtime\n";
    std::cout << " Tree is mutable and tracks the specific state of one instance.\n";
    std::cout << "=================================================================\n";

    // Clean up memory
    delete runtimeRoot; // Recursively deletes the runtime tree
    // The 'def' object goes out of scope here, deleting the Definition Tree.

    return 0;
}*/