
#include <gtest/gtest.h>

#include "ApprovalHandler.h"
#include "TimeoutEscalationHandler.h"
#include "WorkflowStateApprovalHandler.h"
#include "Request.h"
#include "Data.h"

#include <string>

using namespace std;

// Final handler used to verify that requests can
// successfully pass through the approval chain.
class TestFinalHandler : public ApprovalHandler {
public:
    string getName() const override {
        return "TestFinalHandler";
    }

protected:
    HandlerResult handleRequest(Request&) override {
        return HandlerResult::Handled;
    }
};

// Creates a standard request for the tests.
Request makeRequest(const string& action, const string& state) {
    return Request(
        "REQ-1",
        action,
        "WI-1",
        "INST-1",
        "ROLE-1",
        "USER-1",
        "Test reason",
        state,
        Data("test")
    );
}

// Fixture that creates and connects the approval chain.
class ApprovalHandlerTest : public ::testing::Test {
protected:
    TimeoutEscalationHandler timeoutHandler{nullptr};
    WorkflowStateApprovalHandler workflowHandler;
    TestFinalHandler finalHandler;

    void SetUp() override {
        timeoutHandler.setNext(&workflowHandler);
        workflowHandler.setNext(&finalHandler);
    }
};

// Test 1: A timeout request without a WorkItem
// should be rejected by TimeoutEscalationHandler.
TEST_F(ApprovalHandlerTest, RejectsTimeoutWithoutWorkItem) {
    Request request = makeRequest("TIMEOUT", "Available");

    EXPECT_EQ(
        timeoutHandler.authorise(request),
        HandlerResult::Rejected
    );
}

// Test 2: A request in the Created state
// should pass through the chain.
TEST_F(ApprovalHandlerTest, CreatedStatePassesThroughChain) {
    Request request = makeRequest("APPROVE", "Created");

    EXPECT_EQ(
        timeoutHandler.authorise(request),
        HandlerResult::Handled
    );
}

// Test 3: A request in the PickedUp state
// should pass through the chain.
TEST_F(ApprovalHandlerTest, PickedUpStatePassesThroughChain) {
    Request request = makeRequest("APPROVE", "PickedUp");

    EXPECT_EQ(
        timeoutHandler.authorise(request),
        HandlerResult::Handled
    );
}

// Test 4: A request in the In_Progress state
// should be rejected by WorkflowStateApprovalHandler.
TEST_F(ApprovalHandlerTest, RejectsInProgressState) {
    Request request = makeRequest("APPROVE", "In_Progress");

    EXPECT_EQ(
        timeoutHandler.authorise(request),
        HandlerResult::Rejected
    );
}

// Test 5: A request in the Completed state
// should be rejected by WorkflowStateApprovalHandler.
TEST_F(ApprovalHandlerTest, RejectsCompletedState) {
    Request request = makeRequest("APPROVE", "Completed");

    EXPECT_EQ(
        timeoutHandler.authorise(request),
        HandlerResult::Rejected
    );
}

// Test 6: The base handler should reject a request
// when no next handler is connected.
TEST(ApprovalHandlerBaseTest, RejectsWhenNoNextHandlerExists) {
    ApprovalHandler baseHandler;
    Request request = makeRequest("APPROVE", "Created");

    EXPECT_EQ(
        baseHandler.authorise(request),
        HandlerResult::Rejected
    );
}
