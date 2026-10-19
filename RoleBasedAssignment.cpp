/**
 * @file RoleBasedAssignment.cpp
 * @brief Implementation of RoleBasedAssignment (Strategy pattern: ConcreteStrategy).
 */
#include "RoleBasedAssignment.h"
#include "WorkItem.h"

Participant* RoleBasedAssignment::assign(const WorkItem& item,
                                         const std::vector<Participant*>& candidates) {
    const std::string roleId = item.getRequiredRoleId();
    for (Participant* candidate : candidates) {
        if (candidate && candidate->hasRole(roleId)) {
            return candidate;
        }
    }
    return nullptr;
}

std::string RoleBasedAssignment::getName() const { return "RoleBased"; }
