
#include "ApprovalHandler.h"
#include "TimeoutEscalationHandler.h"
#include "WorkflowStateApprovalHandler.h"
#include "Request.h"
#include "Data.h"

#include <iostream>
#include <string>

using namespace std;

// A final test handler that marks a request as handled.
// This lets us test whether Continue passes the request
// through the chain correctly.
class TestFinalHandler : public ApprovalHandler {
public:
    string getName() const override {
        return "TestFinalHandler";
    }

protected:
    HandlerResult handleRequest(Request&) override {
        cout << "Final test handler received the request.\n";
        return HandlerResult::Handled;
    }
};

int passed = 0;
int failed = 0;

void check(const string& testName,
           HandlerResult actual,
           HandlerResult expected) {
    cout << "\nTest: " << testName << "\n";

    if (actual == expected) {
        cout << "PASS\n";
        ++passed;
    } else {
        cout << "FAIL - Expected ";

        if (expected == HandlerResult::Continue)
            cout << "Continue";
        else if (expected == HandlerResult::Handled)
            cout << "Handled";
        else
            cout << "Rejected";

        cout << ", got ";

        if (actual == HandlerResult::Continue)
            cout << "Continue";
        else if (actual == HandlerResult::Handled)
            cout << "Handled";
        else
            cout << "Rejected";

        cout << "\n";
        ++failed;
    }
}

Request makeRequest(const string& action,
                    const string& state) {
    return Request(
        "REQ-1",       // requestId
        action,        // action
        "WI-1",        // workItemId
        "INST-1",      // instanceId
        "ROLE-1",      // requiredRoleId
        "USER-1",      // requesterId
        "Test reason", // reason
        state,         // currentState
        Data("test")   // data
    );
}

int main() {
    cout << "====================================\n";
    cout << "  APPROVAL HANDLER TESTS\n";
    cout << "====================================\n";

    // Create the handlers.
    // nullptr allows us to test the timeout handler's
    // missing-WorkItem rejection path.
    TimeoutEscalationHandler timeoutHandler(nullptr);
    WorkflowStateApprovalHandler workflowHandler;
    TestFinalHandler finalHandler;

    // Connect the handlers in order.
    timeoutHandler.setNext(&workflowHandler);
    workflowHandler.setNext(&finalHandler);

    // Test 1:
    // A timeout with no WorkItem must be rejected.
    {
        Request request = makeRequest("TIMEOUT", "Available");

        check(
            "Timeout without a WorkItem is rejected",
            timeoutHandler.authorise(request),
            HandlerResult::Rejected
        );
    }

    // Test 2:
    // A non-timeout request in Created state should
    // pass both validation handlers and reach the final one.
    {
        Request request = makeRequest("APPROVE", "Created");

        check(
            "Created state passes through the chain",
            timeoutHandler.authorise(request),
            HandlerResult::Handled
        );
    }

    // Test 3:
    // PickedUp is allowed by your current workflow handler.
    {
        Request request = makeRequest("APPROVE", "PickedUp");

        check(
            "PickedUp state passes through the chain",
            timeoutHandler.authorise(request),
            HandlerResult::Handled
        );
    }

    // Test 4:
    // In_Progress is not allowed by the current
    // WorkflowStateApprovalHandler.
    {
        Request request = makeRequest("APPROVE", "In_Progress");

        check(
            "Disallowed state is rejected",
            timeoutHandler.authorise(request),
            HandlerResult::Rejected
        );
    }

    // Test 5:
    // Completed is also disallowed by the workflow handler.
    {
        Request request = makeRequest("APPROVE", "Completed");

        check(
            "Completed state is rejected for approval",
            timeoutHandler.authorise(request),
            HandlerResult::Rejected
        );
    }

    // Test 6:
    // Test the base handler when there is no next handler.
    {
        ApprovalHandler baseHandler;
        Request request = makeRequest("APPROVE", "Created");

        check(
            "Base handler with no next handler rejects",
            baseHandler.authorise(request),
            HandlerResult::Rejected
        );
    }

    cout << "\n====================================\n";
    cout << "TEST SUMMARY\n";
    cout << "Passed: " << passed << "\n";
    cout << "Failed: " << failed << "\n";
    cout << "====================================\n";

    return failed == 0 ? 0 : 1;
}
