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

// Run a command, then show whether it was accepted and what the state is now.
void run(WorkflowAction& cmd, WorkItem& item) {
    std::cout << "\n> " << cmd.getDescription() << "\n";
    bool ok = cmd.execute();
    std::cout << "  result: " << (ok ? "ACCEPTED" : "REJECTED")
              << " | state now: " << item.getCurrentState()->getName() << "\n";
}

int main() {
    ActivityDefinition* def = new ActivityDefinition("act1", "Review", "role1");
    std::vector<std::string> roles;
roles.push_back("role1");
    Participant* person = new Participant("emp1","Thabo",roles,2);
    std::string role = "manager";

    // Scenario 1: normal flow
    std::cout << "=== 1. Normal flow ===\n";
    WorkItem a("item-A", def);
    std::cout << "start state: " << a.getCurrentState()->getName() << "\n";

    a.makeAvailable();                       // Created -> Available
    std::cout << "after makeAvailable: " << a.getCurrentState()->getName() << "\n";

    AssignCommand   assign(&a, person);
    StartCommand    start(&a);
    CompleteCommand complete(&a);
    run(assign, a);                          // Available   -> Assigned
    run(start, a);                           // Assigned    -> In_Progress
    run(complete, a);                        // In_Progress -> Completed

    // Scenario 2: commands rejected by the state 
    std::cout << "\n\n=== 2. Rejected commands ===\n";
    WorkItem b("item-B", def);               // still Created
    StartCommand badStart(&b);
    CompleteCommand badComplete(&b);
    run(badStart, b);                        // can't start before assigning
    run(badComplete, b);                     // can't complete before starting

    CancelCommand cancelA(&a);               // a is already Completed
    run(cancelA, a);                         // terminal state: nothing allowed

    // ---------- Scenario 3: reject, then re-offer ----------
    std::cout << "\n\n=== 3. Reject and re-offer ===\n";
    WorkItem c("item-C", def);
    c.makeAvailable();
    AssignCommand assignC(&c, person);
    RejectCommand rejectC(&c);
    run(assignC, c);                         // -> Assigned
    run(rejectC, c);                         // -> Rejected
    c.makeAvailable();                       // Rejected -> Available
    std::cout << "\nafter makeAvailable: " << c.getCurrentState()->getName() << "\n";

    // ---------- Scenario 4: escalate and cancel ----------
    std::cout << "\n\n=== 4. Escalate then cancel ===\n";
    WorkItem d("item-D", def);
    d.makeAvailable();
    AssignCommand   assignD(&d, person);
    EscalateCommand escalateD(&d, role);
    CancelCommand   cancelD(&d);
    run(assignD, d);                         // -> Assigned
    run(escalateD, d);                       // -> Escalated
    run(cancelD, d);                         // -> Cancelled

   

    delete person;
    delete def;
    return 0;
}