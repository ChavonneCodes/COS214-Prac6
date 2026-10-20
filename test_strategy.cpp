/**
 * @file test_strategy.cpp
 * @brief Functionality test for the Strategy pattern (assignment strategies and WorkItem::assignTo).
 */
// Functionality test for the STRATEGY pattern (AssignmentStrategy family + WorkItem::assignTo).
// Build from the repo folder (one line):
// g++ -std=c++11 -o test_strategy test_strategy.cpp WorkItem.cpp ActivityDefinition.cpp Participant.cpp RoleBasedAssignment.cpp WorkloadBasedAssignment.cpp RuleBasedAssignment.cpp
#include <iostream>
#include <vector>

#include "WorkItem.h"
#include "ActivityDefinition.h"
#include "Participant.h"
#include "AssignmentStrategy.h"
#include "RoleBasedAssignment.h"
#include "WorkloadBasedAssignment.h"
#include "RuleBasedAssignment.h"

static int passed = 0;
static int failed = 0;

#define CHECK(cond, msg)                                                  \
    do {                                                                  \
        if (cond) { ++passed; std::cout << "  [PASS] " << msg << "\n"; }  \
        else      { ++failed; std::cout << "  [FAIL] " << msg << "\n"; }  \
    } while (0)

int main() {
    std::cout << "=== TIVIDY: STRATEGY TESTS ===\n";

    ActivityDefinition def("review", "Technical Review", "team_lead");
    WorkItem item("Claim-2002", &def);
    CHECK(item.getRequiredRoleId() == "team_lead", "WorkItem exposes the role its activity requires");

    std::vector<std::string> proposerRoles;  proposerRoles.push_back("proposer");
    std::vector<std::string> leadRoles;      leadRoles.push_back("team_lead");

    Participant alice("p1", "Alice", proposerRoles, 0);
    Participant bob("p2", "Bob", leadRoles, 1);
    Participant carol("p3", "Carol", leadRoles, 2);

    std::vector<Participant*> candidates;
    candidates.push_back(&alice);
    candidates.push_back(&bob);
    candidates.push_back(&carol);

    // --- RoleBased ---
    RoleBasedAssignment byRole;
    CHECK(byRole.assign(item, candidates) == &bob, "RoleBased skips Alice (wrong role) and picks Bob");
    std::vector<Participant*> onlyAlice;  onlyAlice.push_back(&alice);
    CHECK(byRole.assign(item, onlyAlice) == nullptr, "RoleBased returns nullptr when nobody has the role");
    std::vector<Participant*> nobody;
    CHECK(byRole.assign(item, nobody) == nullptr, "RoleBased returns nullptr for an empty candidate list");

    // --- WorkloadBased ---
    WorkloadBasedAssignment byLoad;
    bob.addOpenItem();
    bob.addOpenItem();    // Bob: 2 open items
    carol.addOpenItem();  // Carol: 1 open item
    Participant* chosen = byLoad.assign(item, candidates);
    CHECK(chosen == &carol, "WorkloadBased picks Carol (fewer open items than Bob)");
    CHECK(chosen != &alice, "WorkloadBased never picks someone without the role");
    carol.addOpenItem();  // now 2 vs 2: tie goes to the earliest candidate
    CHECK(byLoad.assign(item, candidates) == &bob, "WorkloadBased breaks ties in favour of the earliest candidate");
    carol.closeOpenItem();
    carol.closeOpenItem();
    carol.closeOpenItem();  // closing below zero must not go negative
    CHECK(carol.getOpenItems() == 0, "open item count never goes below zero");

    // --- RuleBased (rule supplied from outside) ---
    RuleBasedAssignment bySeniority(
        "level >= 2",
        [](const WorkItem&, const Participant& p) { return p.getLevel() >= 2; });
    CHECK(bySeniority.assign(item, candidates) == &carol, "RuleBased picks the first participant satisfying the rule");
    RuleBasedAssignment impossible(
        "level >= 9",
        [](const WorkItem&, const Participant& p) { return p.getLevel() >= 9; });
    CHECK(impossible.assign(item, candidates) == nullptr, "RuleBased returns nullptr if no one satisfies the rule");
    RuleBasedAssignment noRule("none", RuleBasedAssignment::Rule());
    CHECK(noRule.assign(item, candidates) == nullptr, "RuleBased with an empty rule returns nullptr");

    // --- Interchangeable behind the base type ---
    std::vector<AssignmentStrategy*> strategies;
    strategies.push_back(&byRole);
    strategies.push_back(&byLoad);
    strategies.push_back(&bySeniority);
    std::cout << "  Same item, three strategies:\n";
    for (size_t i = 0; i < strategies.size(); ++i) {
        Participant* p = strategies[i]->assign(item, candidates);
        std::cout << "    " << strategies[i]->getName() << " -> " << (p ? p->getName() : "nobody") << "\n";
    }
    CHECK(strategies.size() == 3, "all strategies used through the AssignmentStrategy interface");

    // --- WorkItem::assignTo ---
    Participant dave("p4", "Dave", leadRoles, 1);
    Participant erin("p5", "Erin", leadRoles, 1);
    CHECK(item.assignTo(&dave), "assignTo succeeds");
    CHECK(item.getState() == "ASSIGNED", "work item state became ASSIGNED");
    CHECK(item.getAssignedParticipant() == &dave, "work item remembers its participant");
    CHECK(dave.getOpenItems() == 1, "participant's open item count went up");
    item.assignTo(&erin);
    CHECK(dave.getOpenItems() == 0 && erin.getOpenItems() == 1, "reassignment moves the open item from Dave to Erin");
    CHECK(!item.assignTo(nullptr), "assigning nobody is rejected");
    CHECK(item.getAssignedParticipant() == &erin, "a rejected assignment leaves the old participant in place");
    item.print();

    std::cout << "\n=== RESULT: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
