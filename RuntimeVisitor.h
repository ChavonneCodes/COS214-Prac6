/**
 * @file RuntimeVisitor.h
 * @brief Operation that can be run over every node of a runtime tree.
 *
 * Part of the **Visitor** pattern (role: Visitor).
 */
#ifndef RUNTIME_VISITOR_H
#define RUNTIME_VISITOR_H

class WorkItem;
class StageInstance;
class SubProcessInstance;

/**
 * @class RuntimeVisitor
 * @brief Declares one visit method per concrete runtime node.
 *
 * A node's accept() calls the visit method for its own class (double
 * dispatch), so a visitor always knows exactly what kind of node it has
 * without any dynamic_cast. New operations over a running workflow (reports,
 * statistics, bulk actions) are added as new visitors, without changing
 * WorkItem, StageInstance or SubProcessInstance.
 *
 * Traversal: a StageInstance calls visitStage(), then passes the visitor to
 * each child in order, then calls leaveStage(). Leaves call only their own
 * visit method.
 */
class RuntimeVisitor {
public:
    virtual ~RuntimeVisitor() = default;

    /**
     * @brief Called for each work item (leaf).
     * @param item The work item being visited.
     */
    virtual void visitWorkItem(WorkItem& item) = 0;

    /**
     * @brief Called when a stage is entered, before its children are visited.
     * @param stage The stage being visited.
     */
    virtual void visitStage(StageInstance& stage) = 0;

    /**
     * @brief Called for each sub-process call (leaf).
     * @param subProcess The sub-process being visited.
     */
    virtual void visitSubProcess(SubProcessInstance& subProcess) = 0;

    /**
     * @brief Called after all of a stage's children have been visited.
     *
     * Does nothing by default. Override it when the visitor needs to know where
     * a stage ends, e.g. to indent a report.
     * @param stage The stage that has just been left.
     */
    virtual void leaveStage(StageInstance& /*stage*/) {}
};

#endif // RUNTIME_VISITOR_H
