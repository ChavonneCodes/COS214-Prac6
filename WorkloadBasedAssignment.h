/**
 * @file WorkloadBasedAssignment.h
 * @brief Assignment that balances workload between qualified participants.
 *
 * Part of the **Strategy** pattern (role: ConcreteStrategy).
 */
#ifndef WORKLOAD_BASED_ASSIGNMENT_H
#define WORKLOAD_BASED_ASSIGNMENT_H

#include "AssignmentStrategy.h"

/**
 * @class WorkloadBasedAssignment
 * @brief Among candidates who hold the required role, picks the one with the
 *        fewest open work items.
 */
class WorkloadBasedAssignment : public AssignmentStrategy {
public:
    /**
     * @brief Returns the least busy candidate who holds the required role.
     * @param item       The work item that needs someone.
     * @param candidates Participants to choose from.
     * @return The least busy qualified participant (ties go to the earliest
     *         candidate), or nullptr if nobody holds the role.
     */
    Participant* assign(const WorkItem& item,
                        const std::vector<Participant*>& candidates) override;

    /**

     * @brief Returns "WorkloadBased".

     * @return "WorkloadBased".

     */
    std::string getName() const override;
};

#endif // WORKLOAD_BASED_ASSIGNMENT_H
