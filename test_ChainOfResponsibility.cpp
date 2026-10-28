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