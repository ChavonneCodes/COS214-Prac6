
/**
 * @file StrategyTest.cpp
 * @brief GoogleTest unit tests for the Strategy pattern.
 */

#include <gtest/gtest.h>

#include <vector>
#include <string>

#include "WorkItem.h"
#include "ActivityDefinition.h"
#include "Participant.h"
#include "AssignmentStrategy.h"
#include "RoleBasedAssignment.h"
#include "WorkloadBasedAssignment.h"
#include "RuleBasedAssignment.h"



// TEST FIXTURE
// Provides reusable definitions, work items and participants.


class StrategyTest : public ::testing::Test {

protected:

    // Activity definition and work item
    ActivityDefinition definition{
        "review", "Technical Review", "team_lead"
    };

    WorkItem item{"Claim-2002", &definition};

    // Participant roles
    std::vector<std::string> proposerRoles{"proposer"};
    std::vector<std::string> leadRoles{"team_lead"};

    // Participants
    Participant alice{"p1", "Alice", proposerRoles, 0};
    Participant bob{"p2", "Bob", leadRoles, 1};
    Participant carol{"p3", "Carol", leadRoles, 2};

    // Candidate collection
    std::vector<Participant*> candidates;


    void SetUp() override {

        candidates.push_back(&alice);
        candidates.push_back(&bob);
        candidates.push_back(&carol);
    }
};



// TEST 1: WORK ITEM REQUIRED ROLE


TEST_F(StrategyTest, RetrievesRequiredRoleFromActivityDefinition) {

    EXPECT_EQ(
        item.getRequiredRoleId(),
        "team_lead"
    );
}



// TEST 2: ROLE-BASED ASSIGNMENT


TEST_F(StrategyTest, SelectsFirstParticipantWithRequiredRole) {

    RoleBasedAssignment strategy;

    Participant* selected = strategy.assign(item, candidates);

    // Alice does not have the required role.
    // Bob is the first participant with the correct role.

    EXPECT_EQ(selected, &bob);
}


TEST_F(StrategyTest, RejectsParticipantsWithoutRequiredRole) {

    RoleBasedAssignment strategy;

    std::vector<Participant*> onlyAlice{&alice};

    EXPECT_EQ(
        strategy.assign(item, onlyAlice),
        nullptr
    );
}


TEST_F(StrategyTest, HandlesEmptyCandidateList) {

    RoleBasedAssignment strategy;

    std::vector<Participant*> nobody;

    EXPECT_EQ(
        strategy.assign(item, nobody),
        nullptr
    );
}



// TEST 3: WORKLOAD-BASED ASSIGNMENT


TEST_F(StrategyTest, SelectsParticipantWithLowestWorkload) {

    WorkloadBasedAssignment strategy;

    // Bob has two open items.
    bob.addOpenItem();
    bob.addOpenItem();

    // Carol has one open item.
    carol.addOpenItem();

    Participant* selected = strategy.assign(item, candidates);

    EXPECT_EQ(selected, &carol);

    // Alice must not be selected because she lacks the role.
    EXPECT_NE(selected, &alice);
}


TEST_F(StrategyTest, BreaksWorkloadTiesUsingCandidateOrder) {

    WorkloadBasedAssignment strategy;

    // Both Bob and Carol have equal workloads.
    bob.addOpenItem();
    bob.addOpenItem();

    carol.addOpenItem();
    carol.addOpenItem();

    Participant* selected = strategy.assign(item, candidates);

    // Bob appears before Carol in the candidate list.
    EXPECT_EQ(selected, &bob);
}


TEST_F(StrategyTest, PreventsOpenItemCountFromGoingBelowZero) {

    // Carol starts with zero open items.
    EXPECT_EQ(carol.getOpenItems(), 0);

    carol.addOpenItem();

    EXPECT_EQ(carol.getOpenItems(), 1);

    carol.closeOpenItem();

    EXPECT_EQ(carol.getOpenItems(), 0);

    // Closing an item when the count is already zero.
    carol.closeOpenItem();

    EXPECT_EQ(carol.getOpenItems(), 0);
}



// TEST 4: RULE-BASED ASSIGNMENT


TEST_F(StrategyTest, SelectsFirstParticipantSatisfyingRule) {

    RuleBasedAssignment strategy(
        "level >= 2",
        [](const WorkItem&, const Participant& participant) {
            return participant.getLevel() >= 2;
        }
    );

    Participant* selected = strategy.assign(item, candidates);

    EXPECT_EQ(selected, &carol);
}


TEST_F(StrategyTest, ReturnsNullptrWhenNoParticipantSatisfiesRule) {

    RuleBasedAssignment strategy(
        "level >= 9",
        [](const WorkItem&, const Participant& participant) {
            return participant.getLevel() >= 9;
        }
    );

    EXPECT_EQ(
        strategy.assign(item, candidates),
        nullptr
    );
}


TEST_F(StrategyTest, ReturnsNullptrWhenRuleIsEmpty) {

    RuleBasedAssignment strategy(
        "none",
        RuleBasedAssignment::Rule()
    );

    EXPECT_EQ(
        strategy.assign(item, candidates),
        nullptr
    );
}



// TEST 5: STRATEGY INTERCHANGEABILITY


TEST_F(StrategyTest, SupportsDifferentStrategiesThroughBaseInterface) {

    RoleBasedAssignment byRole;
    WorkloadBasedAssignment byLoad;

    RuleBasedAssignment bySeniority(
        "level >= 2",
        [](const WorkItem&, const Participant& participant) {
            return participant.getLevel() >= 2;
        }
    );

    // Store different strategy objects using the base type.
    std::vector<AssignmentStrategy*> strategies{
        &byRole,
        &byLoad,
        &bySeniority
    };

    ASSERT_EQ(strategies.size(), 3);

    // Each strategy can be called through the same interface.
    EXPECT_EQ(strategies[0]->assign(item, candidates), &bob);

    EXPECT_EQ(strategies[1]->assign(item, candidates), &bob);

    EXPECT_EQ(strategies[2]->assign(item, candidates), &carol);
}



// TEST 6: WORK ITEM ASSIGNMENT


TEST_F(StrategyTest, SuccessfullyAssignsParticipantToWorkItem) {

    Participant dave("p4", "Dave", leadRoles, 1);

    EXPECT_TRUE(item.assignTo(&dave));

    EXPECT_EQ(item.getState(), "ASSIGNED");

    EXPECT_EQ(item.getAssignedParticipant(), &dave);

    EXPECT_EQ(dave.getOpenItems(), 1);
}



// TEST 7: WORK ITEM REASSIGNMENT


TEST_F(StrategyTest, ReassignmentUpdatesParticipantWorkloads) {

    Participant dave("p4", "Dave", leadRoles, 1);
    Participant erin("p5", "Erin", leadRoles, 1);

    // Initially assign the work item to Dave.
    ASSERT_TRUE(item.assignTo(&dave));

    EXPECT_EQ(dave.getOpenItems(), 1);
    EXPECT_EQ(erin.getOpenItems(), 0);

    // Reassign the work item to Erin.
    ASSERT_TRUE(item.assignTo(&erin));

    EXPECT_EQ(item.getAssignedParticipant(), &erin);

    // Dave's workload decreases.
    EXPECT_EQ(dave.getOpenItems(), 0);

    // Erin's workload increases.
    EXPECT_EQ(erin.getOpenItems(), 1);
}



// TEST 8: INVALID WORK ITEM ASSIGNMENT


TEST_F(StrategyTest, RejectsNullParticipantAssignment) {

    Participant dave("p4", "Dave", leadRoles, 1);

    ASSERT_TRUE(item.assignTo(&dave));

    // Attempt to assign the work item to nobody.
    EXPECT_FALSE(item.assignTo(nullptr));

    // The previous assignment must remain unchanged.
    EXPECT_EQ(item.getAssignedParticipant(), &dave);

    EXPECT_EQ(item.getState(), "ASSIGNED");

    EXPECT_EQ(dave.getOpenItems(), 1);
}
