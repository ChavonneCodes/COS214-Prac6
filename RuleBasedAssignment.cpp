/**
 * @file RuleBasedAssignment.cpp
 * @brief Implementation of RuleBasedAssignment (Strategy pattern: ConcreteStrategy).
 */
#include "RuleBasedAssignment.h"
#include "WorkItem.h"

RuleBasedAssignment::RuleBasedAssignment(const std::string& description, const Rule& rule)
    : description(description), rule(rule) {}

Participant* RuleBasedAssignment::assign(const WorkItem& item,
                                         const std::vector<Participant*>& candidates) {
    if (!rule) return nullptr;
    for (Participant* candidate : candidates) {
        if (candidate && rule(item, *candidate)) {
            return candidate;
        }
    }
    return nullptr;
}

std::string RuleBasedAssignment::getName() const { return "RuleBased(" + description + ")"; }
