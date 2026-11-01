
/**
 * @file RoutingStrategyTest.cpp
 * @brief GoogleTest functionality tests for Sequential and Conditional Routing.
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "WorkflowDefinition.h"
#include "ActivityDefinition.h"
#include "TransitionDefinition.h"
#include "SequentialRouting.h"
#include "ConditionalRouting.h"
#include "RoutingStrategy.h"

class RoutingStrategyTest : public ::testing::Test {
protected:
    WorkflowDefinition def{"wf1", "Leave request", 1};

    ActivityDefinition a{"A", "Submit", "r1"};
    ActivityDefinition b{"B", "Review", "r2"};
    ActivityDefinition c{"C", "Approve", "r3"};
    ActivityDefinition d{"D", "Reject", "r3"};

    SequentialRouting seq;
    ConditionalRouting cond{"passed", "C", "D"};

    RoutingStrategy* strategy;

    void SetUp() override {
        def.addActivity(a);
        def.addActivity(b);
        def.addActivity(c);
        def.addActivity(d);

        def.addTransition(
            TransitionDefinition("A", "B", "always")
        );

        def.addTransition(
            TransitionDefinition("B", "C", "passed")
        );

        strategy = &seq;
    }
};

// 1. Sequential routing strategy name
TEST_F(RoutingStrategyTest, SequentialRoutingReturnsCorrectName) {
    EXPECT_EQ(strategy->getName(), "Sequential");
}

// 2. Sequential routing returns one next activity
TEST_F(RoutingStrategyTest, SequentialRoutingReturnsOneActivity) {
    std::vector<const ActivityDefinition*> result =
        strategy->next(a, def, "");

    EXPECT_EQ(result.size(), 1);
}

// 3. Sequential routing moves from A to B
TEST_F(RoutingStrategyTest, SequentialRoutingMovesToNextActivity) {
    std::vector<const ActivityDefinition*> result =
        strategy->next(a, def, "");

    ASSERT_EQ(result.size(), 1);
    ASSERT_NE(result[0], nullptr);

    EXPECT_EQ(result[0]->getId(), "B");
}

// 4. Sequential routing ignores the outcome
TEST_F(RoutingStrategyTest, SequentialRoutingIgnoresOutcome) {
    std::vector<const ActivityDefinition*> result =
        strategy->next(a, def, "failed");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->getId(), "B");
}

// 5. Sequential routing does not follow conditional transitions
TEST_F(RoutingStrategyTest, SequentialRoutingIgnoresConditionalTransition) {
    std::vector<const ActivityDefinition*> result =
        strategy->next(b, def, "");

    EXPECT_TRUE(result.empty());
}

// 6. Sequential routing returns empty for a dead end
TEST_F(RoutingStrategyTest, SequentialRoutingReturnsEmptyForDeadEnd) {
    std::vector<const ActivityDefinition*> result =
        strategy->next(d, def, "");

    EXPECT_TRUE(result.empty());
}

// 7. Conditional routing strategy name
TEST_F(RoutingStrategyTest, ConditionalRoutingReturnsCorrectName) {
    RoutingStrategy* strategy = &cond;

    EXPECT_EQ(strategy->getName(), "Conditional(passed)");
}

// 8. Conditional routing selects C when outcome is passed
TEST_F(RoutingStrategyTest, ConditionalRoutingSelectsApprovalActivity) {
    std::vector<const ActivityDefinition*> result =
        cond.next(b, def, "passed");

    ASSERT_EQ(result.size(), 1);
    ASSERT_NE(result[0], nullptr);

    EXPECT_EQ(result[0]->getId(), "C");
}

// 9. Conditional routing selects D when outcome is failed
TEST_F(RoutingStrategyTest, ConditionalRoutingSelectsRejectionActivity) {
    std::vector<const ActivityDefinition*> result =
        cond.next(b, def, "failed");

    ASSERT_EQ(result.size(), 1);
    ASSERT_NE(result[0], nullptr);

    EXPECT_EQ(result[0]->getId(), "D");
}

// 10. Conditional routing handles unknown target activities
TEST_F(RoutingStrategyTest, ConditionalRoutingReturnsEmptyForUnknownTarget) {
    ConditionalRouting bad("passed", "X", "Y");

    std::vector<const ActivityDefinition*> result =
        bad.next(b, def, "passed");

    EXPECT_TRUE(result.empty());
}
