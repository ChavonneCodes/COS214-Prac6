#include <iostream>

#include "Request.h"
#include "ValidationApprovalHandler.h"
#include "WorkflowStateApprovalHandler.h"
#include "RoleApprovalHandler.h"
#include "TimeoutEscalationHandler.h"

using namespace std;

int main() {
    ValidationApprovalHandler validation;
    WorkflowStateApprovalHandler stateHandler;
    RoleApprovalHandler managerHandler("MANAGER");
    TimeoutEscalationHandler timeoutHandler;

    // Build the chain.
    validation.setNext(&stateHandler);
    stateHandler.setNext(&managerHandler);
    managerHandler.setNext(&timeoutHandler);

    Data data("Purchase of new equipment");

    Request request(
        "REQ-001",
        "APPROVE",
        "WI-001",
        "WF-100",
        "MANAGER",
        "EMP-012",
        "Equipment purchase approval",
        "Created",
        data
    );

    HandlerResult result = validation.authorise(request);

    if (result == HandlerResult::Handled) {
        cout << "\nRequest processed successfully.\n";
    } else if (result == HandlerResult::Rejected) {
        cout << "\nRequest rejected or could not be processed.\n";
    }

    return 0;
}

/*
#include <iostream>
#include "Request.h"
#include "RoleApprovalHandler.h"
#include "TimeoutEscalationHandler.h"

int main() {
    RoleApprovalHandler roleHandler("MANAGER");
    TimeoutEscalationHandler timeoutHandler;

    roleHandler.setNext(&timeoutHandler);

    Request request1("APPROVE", "WI-001", "WF-100", "MANAGER","1");
    Request request2("TIMEOUT", "WI-002", "WF-100", "SUPERVISOR","2");
    Request request3("REJECT", "WI-003", "WF-101", "EMPLOYEE","3");

    std::cout << "Request 1: "
              << roleHandler.authorise(request1) << '\n';

    std::cout << "Request 2: "
              << roleHandler.authorise(request2) << '\n';

    std::cout << "Request 3: "
              << roleHandler.authorise(request3) << '\n';

    return 0;
}
*/