/**
 * @file RuleBasedAssignment.h
 * @brief Assignment decided by a rule supplied from outside.
 *
 * Part of the **Strategy** pattern (role: ConcreteStrategy).
 */
#ifndef RULE_BASED_ASSIGNMENT_H
#define RULE_BASED_ASSIGNMENT_H

#include <functional>
#include "AssignmentStrategy.h"

/**
 * @class RuleBasedAssignment
 * @brief Uses a predicate to decide who may take a work item.
 *
 * The rule can depend on the work item and the participant, for example
 * "only participants of level 2 or higher may take this item". The first
 * candidate who satisfies the rule is chosen.
 */
class RuleBasedAssignment : public AssignmentStrategy {
public:
    /** @brief Predicate type: returns true if the participant may take the item. */
    typedef std::function<bool(const WorkItem&, const Participant&)> Rule;

private:
    std::string description; ///< Short text describing the rule (used in getName()).
    Rule rule;               ///< The rule itself.

public:
    /**
     * @brief Creates a strategy from a rule.
     * @param description Short text describing the rule, e.g. "level >= 2".
     * @param rule        Predicate deciding whether a participant may take an item.
     */
    RuleBasedAssignment(const std::string& description, const Rule& rule);

    /**
     * @brief Returns the first candidate that satisfies the rule.
     * @param item       The work item that needs someone.
     * @param candidates Participants to choose from.
     * @return The first participant for whom the rule is true, or nullptr if
     *         none qualifies or the rule is empty.
     */
    Participant* assign(const WorkItem& item,
                        const std::vector<Participant*>& candidates) override;

    /**

     * @brief Returns "RuleBased(" followed by the rule description and ")".

     * @return "RuleBased(" followed by the rule description and ")".

     */
    std::string getName() const override;
};

#endif // RULE_BASED_ASSIGNMENT_H
