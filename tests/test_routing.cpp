/**
 * @file test_routing.cpp
 * @brief GoogleTest unit tests for the routing Strategy pattern
 *        (SequentialRouting, ConditionalRouting).
 *
 * Covers transitions (which step follows which) and invalid input (unknown ids,
 * dead ends).
 */
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "ActivityDefinition.h"
#include "ConditionalRouting.h"
#include "SequentialRouting.h"
#include "TransitionDefinition.h"
#include "WorkflowDefinition.h"

namespace {

std::vector<std::string> ids(const std::vector<const WorkflowComponent*>& nodes) {
    std::vector<std::string> out;
    for (size_t i = 0; i < nodes.size(); ++i) out.push_back(nodes[i]->getId());
    return out;
}

}  // namespace

/**
 * A -(always)-> B -(passed)-> C, and D is the "rejected" branch.
 * C and D have no outgoing transitions.
 */
class RoutingTest : public ::testing::Test {
protected:
    WorkflowDefinition def{"wf1", "Leave request", 1};
    ActivityDefinition a{"A", "Submit", "r1"};
    ActivityDefinition b{"B", "Review", "r2"};
    ActivityDefinition c{"C", "Approve", "r3"};
    ActivityDefinition d{"D", "Reject", "r3"};

    void SetUp() override {
        def.addActivity(a);
        def.addActivity(b);
        def.addActivity(c);
        def.addActivity(d);
        def.addTransition(TransitionDefinition("A", "B", "always"));
        def.addTransition(TransitionDefinition("B", "C", "passed"));
    }
};

//  SequentialRouting 

TEST_F(RoutingTest, SequentialHasAName) {
    SequentialRouting routing;
    EXPECT_EQ(routing.getName(), "Sequential");
}

TEST_F(RoutingTest, SequentialFollowsAnAlwaysTransition) {
    SequentialRouting routing;
    EXPECT_EQ(ids(routing.next(a, def, "")), std::vector<std::string>(1, "B"));
}

TEST_F(RoutingTest, SequentialIgnoresTheOutcome) {
    SequentialRouting routing;
    EXPECT_EQ(ids(routing.next(a, def, "failed")), ids(routing.next(a, def, "passed")));
}

TEST_F(RoutingTest, SequentialTreatsAConditionalOnlyStepAsADeadEnd) {
    SequentialRouting routing;
    EXPECT_TRUE(routing.next(b, def, "passed").empty());
}

TEST_F(RoutingTest, SequentialTreatsAStepWithNoTransitionsAsADeadEnd) {
    SequentialRouting routing;
    EXPECT_TRUE(routing.next(d, def, "").empty());
}

TEST_F(RoutingTest, SequentialFansOutToSeveralStepsInOrder) {
    SequentialRouting routing;
    def.addTransition(TransitionDefinition("A", "D", "always"));
    std::vector<std::string> next = ids(routing.next(a, def, ""));
    ASSERT_EQ(next.size(), 2u);
    EXPECT_EQ(next[0], "B");
    EXPECT_EQ(next[1], "D");
}

TEST_F(RoutingTest, SequentialSkipsATransitionToAnUnknownActivity) {
    SequentialRouting routing;
    def.addTransition(TransitionDefinition("C", "ghost", "always"));
    EXPECT_TRUE(routing.next(c, def, "").empty());
}

//  ConditionalRouting 

TEST_F(RoutingTest, ConditionalHasAName) {
    ConditionalRouting routing("passed", "C", "D");
    EXPECT_EQ(routing.getName(), "Conditional(passed)");
}

TEST_F(RoutingTest, ConditionalMatchingOutcomeGoesToTheTrueTarget) {
    ConditionalRouting routing("passed", "C", "D");
    EXPECT_EQ(ids(routing.next(b, def, "passed")), std::vector<std::string>(1, "C"));
}

TEST_F(RoutingTest, ConditionalAnyOtherOutcomeGoesToTheFalseTarget) {
    ConditionalRouting routing("passed", "C", "D");
    EXPECT_EQ(ids(routing.next(b, def, "failed")), std::vector<std::string>(1, "D"));
}

TEST_F(RoutingTest, ConditionalEmptyOutcomeTakesTheFalseBranch) {
    ConditionalRouting routing("passed", "C", "D");
    EXPECT_EQ(ids(routing.next(b, def, "")), std::vector<std::string>(1, "D"));
}

TEST_F(RoutingTest, ConditionalWithTargetsThatDoNotExistGivesNoNextStep) {
    ConditionalRouting broken("passed", "X", "Y");
    EXPECT_TRUE(broken.next(b, def, "passed").empty());
    EXPECT_TRUE(broken.next(b, def, "failed").empty());
}

//  Strategies are interchangeable 

TEST_F(RoutingTest, StrategiesCanBeSwappedAtRuntime) {
    SequentialRouting sequential;
    ConditionalRouting conditional("passed", "C", "D");

    RoutingStrategy* routing = &sequential;
    EXPECT_EQ(ids(routing->next(a, def, "")), std::vector<std::string>(1, "B"));

    routing = &conditional;  // same calling code, different rule
    EXPECT_EQ(ids(routing->next(b, def, "failed")), std::vector<std::string>(1, "D"));
}

TEST_F(RoutingTest, RoutingNeverChangesTheDefinition) {
    SequentialRouting routing;
    routing.next(a, def, "");
    routing.next(b, def, "");
    EXPECT_EQ(def.getTransitions().size(), 2u);
    EXPECT_EQ(def.getActivities().size(), 4u);
    EXPECT_EQ(def.getVersion(), 1);
}
