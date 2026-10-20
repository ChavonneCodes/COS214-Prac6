/**
 * @file AssignmentStrategy.h
 * @brief Abstract rule for choosing who receives a work item.
 *
 * Part of the **Strategy** pattern (role: Strategy).
 */
#ifndef ASSIGNMENT_STRATEGY_H
#define ASSIGNMENT_STRATEGY_H

#include <string>
#include <vector>
#include "Participant.h"

class WorkItem;

/**
 * @class AssignmentStrategy
 * @brief Decides which participant should receive a work item.
 *
 * Different organisations pick different rules (by role, by workload, by a
 * calculated rule). Each rule is a subclass, so the work-item classes never
 * change when the rule does.
 */
class AssignmentStrategy {
public:
    virtual ~AssignmentStrategy() = default;

    /**
     * @brief Chooses a participant for a work item.
     * @param item       The work item that needs someone.
     * @param candidates Participants to choose from (not owned, may be empty).
     * @return The chosen participant, or nullptr if nobody is suitable.
     */
    virtual Participant* assign(const WorkItem& item,
                                const std::vector<Participant*>& candidates) = 0;

    /**

     * @brief Returns a short name describing the strategy.

     * @return A short name describing the strategy.

     */
    virtual std::string getName() const = 0;
};

#endif // ASSIGNMENT_STRATEGY_H
