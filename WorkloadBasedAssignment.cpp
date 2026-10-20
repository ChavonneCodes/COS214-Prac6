/**
 * @file WorkloadBasedAssignment.cpp
 * @brief Implementation of WorkloadBasedAssignment (Strategy pattern: ConcreteStrategy).
 */
#include "WorkloadBasedAssignment.h"
#include "WorkItem.h"

Participant* WorkloadBasedAssignment::assign(const WorkItem& item,
                                             const std::vector<Participant*>& candidates) {
    const std::string roleId = item.getRequiredRoleId();
    Participant* best = nullptr;
    for (Participant* candidate : candidates) {
        if (!candidate || !candidate->hasRole(roleId)) continue;
        if (best == nullptr || candidate->getOpenItems() < best->getOpenItems()) {
            best = candidate;
        }
    }
    return best;
}

std::string WorkloadBasedAssignment::getName() const { return "WorkloadBased"; }
