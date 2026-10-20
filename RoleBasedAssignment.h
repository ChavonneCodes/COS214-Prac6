/**
 * @file RoleBasedAssignment.h
 * @brief Assignment by required role.
 *
 * Part of the **Strategy** pattern (role: ConcreteStrategy).
 */
#ifndef ROLE_BASED_ASSIGNMENT_H
#define ROLE_BASED_ASSIGNMENT_H

#include "AssignmentStrategy.h"

/**
 * @class RoleBasedAssignment
 * @brief Picks the first candidate who holds the role the activity requires.
 */
class RoleBasedAssignment : public AssignmentStrategy {
public:
    /**
     * @brief Returns the first candidate holding the work item's required role.
     * @param item       The work item that needs someone.
     * @param candidates Participants to choose from.
     * @return The first matching participant, or nullptr if none holds the role.
     */
    Participant* assign(const WorkItem& item,
                        const std::vector<Participant*>& candidates) override;

    /**

     * @brief Returns "RoleBased".

     * @return "RoleBased".

     */
    std::string getName() const override;
};

#endif // ROLE_BASED_ASSIGNMENT_H
