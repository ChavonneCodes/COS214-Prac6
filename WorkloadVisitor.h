/**
 * @file WorkloadVisitor.h
 * @brief Visitor that groups the work items of a runtime tree by participant.
 *
 * Part of the **Visitor** pattern (role: ConcreteVisitor).
 */
#ifndef WORKLOAD_VISITOR_H
#define WORKLOAD_VISITOR_H

#include <map>
#include <string>
#include <vector>
#include "RuntimeVisitor.h"

/**
 * @class WorkloadVisitor
 * @brief Shows who holds which work items and how many are still open.
 *
 * An item is "open" unless it is Completed or Cancelled. Stages and
 * sub-processes are not assigned to anyone, so they are skipped.
 */
class WorkloadVisitor : public RuntimeVisitor {
private:
    std::vector<std::string> participants;                       ///< Participant names, in the order first seen.
    std::map<std::string, std::vector<std::string>> itemsFor;    ///< Participant name -> names of their items.
    std::map<std::string, int> openFor;                          ///< Participant name -> number of open items.
    std::vector<std::string> unassigned;                         ///< Names of items nobody holds.

public:
    /**
     * @brief Adds the item to its participant's list (or to the unassigned list).
     * @param item The work item being visited.
     */
    void visitWorkItem(WorkItem& item) override;

    /** @brief Does nothing: stages are not assigned to anyone. */
    void visitStage(StageInstance& stage) override;

    /** @brief Does nothing: sub-processes are not assigned to anyone. */
    void visitSubProcess(SubProcessInstance& subProcess) override;

    /**
     * @brief Returns every participant that holds at least one item.
     * @return Participant names, in the order they were first seen.
     */
    std::vector<std::string> getParticipants() const;

    /**
     * @brief Returns the items a participant holds.
     * @param participantName Name of the participant.
     * @return Item names, in tree order (empty if they hold none).
     */
    std::vector<std::string> getItemsFor(const std::string& participantName) const;

    /**
     * @brief Returns how many of a participant's items are still open.
     * @param participantName Name of the participant.
     * @return The number of items that are not Completed or Cancelled.
     */
    int getOpenCount(const std::string& participantName) const;

    /**
     * @brief Returns the items nobody is assigned to.
     * @return Item names, in tree order.
     */
    std::vector<std::string> getUnassigned() const;

    /**
     * @brief Returns one summary line per participant, then the unassigned items.
     * @return The summary lines.
     */
    std::vector<std::string> getSummary() const;
};

#endif // WORKLOAD_VISITOR_H
