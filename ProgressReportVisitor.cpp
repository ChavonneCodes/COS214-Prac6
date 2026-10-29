/**
 * @file ProgressReportVisitor.cpp
 * @brief Implementation of ProgressReportVisitor (Visitor pattern: ConcreteVisitor).
 */
#include "ProgressReportVisitor.h"
#include "WorkItem.h"
#include "StageInstance.h"
#include "SubProcessInstance.h"

void ProgressReportVisitor::addLine(const std::string& text) {
    lines.push_back(std::string(depth * 2, ' ') + text);
}

void ProgressReportVisitor::visitWorkItem(WorkItem& item) {
    std::string state = item.getState();
    ++totalItems;
    ++stateCounts[state];
    if (state == "Completed") ++completedItems;

    Participant* who = item.getAssignedParticipant();
    addLine("[" + state + "] " + item.getName() + " (" + (who ? who->getName() : "unassigned") + ")");
}

void ProgressReportVisitor::visitStage(StageInstance& stage) {
    addLine("Stage: " + stage.getName());
    ++depth;
}

void ProgressReportVisitor::visitSubProcess(SubProcessInstance& subProcess) {
    addLine("[" + subProcess.getState() + "] sub-process " + subProcess.getId());
}

void ProgressReportVisitor::leaveStage(StageInstance& /*stage*/) {
    if (depth > 0) --depth;
}

int ProgressReportVisitor::getTotalItems() const { return totalItems; }

int ProgressReportVisitor::getCompletedItems() const { return completedItems; }

int ProgressReportVisitor::getPercentComplete() const {
    return totalItems == 0 ? 0 : completedItems * 100 / totalItems;
}

bool ProgressReportVisitor::isComplete() const {
    return totalItems > 0 && completedItems == totalItems;
}

std::map<std::string, int> ProgressReportVisitor::getStateCounts() const { return stateCounts; }

std::vector<std::string> ProgressReportVisitor::getReport() const {
    std::vector<std::string> report = lines;
    report.push_back(std::to_string(completedItems) + "/" + std::to_string(totalItems) +
                     " work items completed (" + std::to_string(getPercentComplete()) + "%)");
    return report;
}
