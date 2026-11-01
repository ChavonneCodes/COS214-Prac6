
/**
 * @file RuntimeDecoratorTest.cpp
 * @brief GoogleTest functionality tests for Runtime Decorators.
 */

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "StageDefinition.h"
#include "StageInstance.h"
#include "Participant.h"
#include "RuntimeComponentDecorator.h"
#include "SecurityRuntimeDecorator.h"
#include "AllowedStatesRuntimeDecorator.h"
#include "LoggingRuntimeDecorator.h"
#include "LabelledRuntimeDecorator.h"

// Captures cout output so logging and labels can be tested.
class CaptureOutput {
private:
    std::ostringstream output;
    std::streambuf* previous;

public:
    CaptureOutput()
        : previous(std::cout.rdbuf(output.rdbuf())) {}

    ~CaptureOutput() {
        std::cout.rdbuf(previous);
    }

    std::string text() const {
        return output.str();
    }
};

// 1. Security decorator tests
TEST(RuntimeDecoratorTest, SecurityRejectsUnauthorizedUpdates) {
    StageDefinition definition("review", "Review stage");

    Participant reviewer("p1", "Alice", {"reviewer"});
    Participant visitor("p2", "Visitor", {"guest"});

    SecurityRuntimeDecorator stage(
        new StageInstance("case-1", &definition),
        "reviewer"
    );

    // No actor should be rejected.
    EXPECT_THROW(stage.setState("RUNNING"), std::runtime_error);
    EXPECT_EQ(stage.getState(), "IDLE");

    // An actor with the wrong role should be rejected.
    stage.setActor(&visitor);

    EXPECT_THROW(stage.setState("RUNNING"), std::runtime_error);
    EXPECT_FALSE(stage.canModify());
    EXPECT_EQ(stage.getState(), "IDLE");

    // An actor with the correct role should be accepted.
    stage.setActor(&reviewer);
    stage.setState("RUNNING");

    EXPECT_TRUE(stage.canModify());
    EXPECT_EQ(stage.getState(), "RUNNING");

    // Removing the actor should revoke access.
    stage.setActor(nullptr);

    EXPECT_THROW(stage.setState("COMPLETED"), std::runtime_error);
    EXPECT_EQ(stage.getState(), "RUNNING");
}

TEST(RuntimeDecoratorTest, SecurityRejectsUnauthorizedChildInsertion) {
    StageDefinition definition("review", "Review stage");
    StageDefinition childDefinition("child", "Child stage");

    Participant reviewer("p1", "Alice", {"reviewer"});
    Participant visitor("p2", "Visitor", {"guest"});

    SecurityRuntimeDecorator stage(
        new StageInstance("case-1", &definition),
        "reviewer"
    );

    stage.setActor(&visitor);

    StageInstance* child =
        new StageInstance("case-1", &childDefinition);

    EXPECT_THROW(stage.addChild(child), std::runtime_error);
    EXPECT_TRUE(stage.getChildren().empty());

    // The rejected child was not transferred to the decorator.
    delete child;

    // Authorized insertion should succeed.
    stage.setActor(&reviewer);

    stage.addChild(
        new StageInstance("case-1", &childDefinition)
    );

    EXPECT_EQ(stage.getChildren().size(), 1);
}

// 2. Allowed states decorator tests
TEST(RuntimeDecoratorTest, AllowedStatesRejectsUnknownState) {
    StageDefinition definition("review", "Review stage");

    AllowedStatesRuntimeDecorator stage(
        new StageInstance("case-1", &definition),
        {"IDLE", "RUNNING", "COMPLETED"}
    );

    EXPECT_THROW(stage.setState("BANANA"), std::invalid_argument);
    EXPECT_EQ(stage.getState(), "IDLE");
}

TEST(RuntimeDecoratorTest, AllowedStatesAcceptsValidState) {
    StageDefinition definition("review", "Review stage");

    AllowedStatesRuntimeDecorator stage(
        new StageInstance("case-1", &definition),
        {"IDLE", "RUNNING", "COMPLETED"}
    );

    stage.setState("RUNNING");

    EXPECT_EQ(stage.getState(), "RUNNING");
}

// 3. Logging decorator tests
TEST(RuntimeDecoratorTest, LoggingForwardsStateUpdate) {
    StageDefinition definition("review", "Review stage");

    LoggingRuntimeDecorator stage(
        new StageInstance("case-1", &definition)
    );

    std::string log;

    {
        CaptureOutput capture;

        stage.setState("RUNNING");

        log = capture.text();
    }

    EXPECT_EQ(stage.getState(), "RUNNING");

    EXPECT_EQ(
        log,
        "[STATE] review: IDLE -> RUNNING\n"
    );
}

// 4. Label decorator tests
TEST(RuntimeDecoratorTest, LabelAddsHeadingToPrintedOutput) {
    StageDefinition definition("review", "Review stage");

    LabelledRuntimeDecorator stage(
        new StageInstance("case-1", &definition),
        "Campus incident"
    );

    std::string printed;

    {
        CaptureOutput capture;

        stage.print();

        printed = capture.text();
    }

    EXPECT_EQ(
        printed,
        "[Campus incident]\n"
        "  [Stage] Review stage (State: IDLE)\n"
    );
}

TEST(RuntimeDecoratorTest, LabelPreservesComponentInformation) {
    StageDefinition definition("review", "Review stage");

    LabelledRuntimeDecorator stage(
        new StageInstance("case-1", &definition),
        "Campus incident"
    );

    EXPECT_EQ(stage.getId(), "review");
    EXPECT_EQ(stage.getState(), "IDLE");
}

// 5. Stacked decorators tests
TEST(RuntimeDecoratorTest, StackedDecoratorsRejectInvalidRequests) {
    StageDefinition definition("review", "Review stage");

    Participant reviewer("p1", "Alice", {"reviewer"});
    Participant visitor("p2", "Visitor", {"guest"});

    SecurityRuntimeDecorator stage(
        new AllowedStatesRuntimeDecorator(
            new LoggingRuntimeDecorator(
                new StageInstance("case-1", &definition)
            ),
            {"IDLE", "RUNNING"}
        ),
        "reviewer",
        &visitor
    );

    bool accessDenied = false;
    bool invalidState = false;

    std::string log;

    {
        CaptureOutput capture;

        // Rejected by Security decorator.
        EXPECT_THROW(
            stage.setState("RUNNING"),
            std::runtime_error
        );

        accessDenied = true;

        // Correct role, but invalid state.
        stage.setActor(&reviewer);

        EXPECT_THROW(
            stage.setState("BANANA"),
            std::invalid_argument
        );

        invalidState = true;

        log = capture.text();
    }

    EXPECT_TRUE(accessDenied);
    EXPECT_TRUE(invalidState);
    EXPECT_EQ(stage.getState(), "IDLE");

    // Rejected requests should never reach Logging.
    EXPECT_TRUE(log.empty());
}

TEST(RuntimeDecoratorTest, StackedDecoratorsAllowValidRequests) {
    StageDefinition definition("review", "Review stage");

    Participant reviewer("p1", "Alice", {"reviewer"});

    SecurityRuntimeDecorator stage(
        new AllowedStatesRuntimeDecorator(
            new LoggingRuntimeDecorator(
                new StageInstance("case-1", &definition)
            ),
            {"IDLE", "RUNNING"}
        ),
        "reviewer",
        &reviewer
    );

    std::string log;

    {
        CaptureOutput capture;

        stage.setState("RUNNING");

        log = capture.text();
    }

    EXPECT_EQ(stage.getState(), "RUNNING");

    EXPECT_EQ(
        log,
        "[STATE] review: IDLE -> RUNNING\n"
    );
}

// 6. Construction tests
TEST(RuntimeDecoratorTest, RejectsNullComponent) {
    EXPECT_THROW(
        RuntimeComponentDecorator(nullptr),
        std::invalid_argument
    );
}
