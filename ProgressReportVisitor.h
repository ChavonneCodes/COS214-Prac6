/**
 * @file ProgressReportVisitor.h
 * @brief Visitor that builds a progress / completion report of a runtime tree.
 *
 * Part of the **Visitor** pattern (role: ConcreteVisitor). Implements the
 * "Generate completion report" step of Activity Diagram 1.
 */
#ifndef PROGRESS_REPORT_VISITOR_H
#define PROGRESS_REPORT_VISITOR_H

#include <map>
#include <string>
#include <vector>
#include "RuntimeVisitor.h"

/**
 * @class ProgressReportVisitor
 * @brief Lists every stage and work item with its state, and counts progress.
 *
 * Use a new visitor for each report: counts and lines add up across visits.
 */
class ProgressReportVisitor : public RuntimeVisitor {
private:
    std::vector<std::string> lines;          ///< One indented line per node, in tree order.
    std::map<std::string, int> stateCounts;  ///< Number of work items in each state.
    int depth = 0;                           ///< Current stage nesting, used for indentation.
    int totalItems = 0;                      ///< Work items seen.
    int completedItems = 0;                  ///< Work items in the Completed state.

    /**
     * @brief Adds a line indented to the current depth.
     * @param text The line to add.
     */
    void addLine(const std::string& text);

public:
    /**
     * @brief Records the item's state and who it is assigned to.
     * @param item The work item being visited.
     */
    void visitWorkItem(WorkItem& item) override;

    /**
     * @brief Starts a new section for the stage and indents what follows.
     * @param stage The stage being entered.
     */
    void visitStage(StageInstance& stage) override;

    /**
     * @brief Records the sub-process call and its state (not counted as a work item).
     * @param subProcess The sub-process being visited.
     */
    void visitSubProcess(SubProcessInstance& subProcess) override;

    /**
     * @brief Ends the stage's section.
     * @param stage The stage being left.
     */
    void leaveStage(StageInstance& stage) override;

    /**
     * @brief Returns how many work items were visited.
     * @return The number of work items.
     */
    int getTotalItems() const;

    /**
     * @brief Returns how many visited work items are Completed.
     * @return The number of completed work items.
     */
    int getCompletedItems() const;

    /**
     * @brief Returns the completed share of work items, rounded down.
     * @return 0 to 100 (0 if there are no work items).
     */
    int getPercentComplete() const;

    /**
     * @brief Checks whether every work item is Completed.
     * @return true if there is at least one work item and all are Completed.
     */
    bool isComplete() const;

    /**
     * @brief Returns how many work items are in each state.
     * @return Map from state name (e.g. "Completed") to count.
     */
    std::map<std::string, int> getStateCounts() const;

    /**
     * @brief Returns the report: one line per node, then a summary line.
     * @return The report lines, in tree order.
     */
    std::vector<std::string> getReport() const;
};

#endif // PROGRESS_REPORT_VISITOR_H
