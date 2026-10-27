/**
 * @file WorkloadVisitor.cpp
 * @brief Implementation of WorkloadVisitor (Visitor pattern: ConcreteVisitor).
 */
#include "WorkloadVisitor.h"
#include "WorkItem.h"

void WorkloadVisitor::visitWorkItem(WorkItem& item) {
    Participant* who = item.getAssignedParticipant();
    if (!who) {
        unassigned.push_back(item.getName());
        return;
    }

    std::string name = who->getName();
    if (itemsFor.find(name) == itemsFor.end()) participants.push_back(name);
    itemsFor[name].push_back(item.getName());

    std::string state = item.getState();
    if (state != "Completed" && state != "Cancelled") ++openFor[name];
}

void WorkloadVisitor::visitStage(StageInstance& /*stage*/) {}

void WorkloadVisitor::visitSubProcess(SubProcessInstance& /*subProcess*/) {}

std::vector<std::string> WorkloadVisitor::getParticipants() const { return participants; }

std::vector<std::string> WorkloadVisitor::getItemsFor(const std::string& participantName) const {
    auto found = itemsFor.find(participantName);
    return found == itemsFor.end() ? std::vector<std::string>() : found->second;
}

int WorkloadVisitor::getOpenCount(const std::string& participantName) const {
    auto found = openFor.find(participantName);
    return found == openFor.end() ? 0 : found->second;
}

std::vector<std::string> WorkloadVisitor::getUnassigned() const { return unassigned; }

std::vector<std::string> WorkloadVisitor::getSummary() const {
    std::vector<std::string> summary;
    for (const std::string& name : participants) {
        std::string line = name + ": " + std::to_string(getOpenCount(name)) + " open of " +
                           std::to_string(itemsFor.at(name).size()) + " (";
        const std::vector<std::string>& items = itemsFor.at(name);
        for (size_t i = 0; i < items.size(); ++i) {
            line += (i ? ", " : "") + items[i];
        }
        summary.push_back(line + ")");
    }
    if (!unassigned.empty()) {
        summary.push_back("Unassigned: " + std::to_string(unassigned.size()) + " item(s)");
    }
    return summary;
}
