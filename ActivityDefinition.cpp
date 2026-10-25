#include "ActivityDefinition.h"
/**
 * @class ActivityDefinition
 * @brief Represents a single leaf node (activity) in the static Workflow Definition tree.
 * 
 * Part of the Composite pattern, this class defines a specific piece of work
 * that must be performed within a workflow.
 */

ActivityDefinition::ActivityDefinition(const std::string& id, const std::string& name, 
                       const std::string& roleId, const std::string& chainId)
    : activityId(id), name(name), requiredRoleId(roleId), approvalChainId(chainId) {}

std::string ActivityDefinition::getApprovalChainId() const { return approvalChainId; }
void ActivityDefinition::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "- Activity: " << name << " (ID: " << activityId << ", Role: " << requiredRoleId << ")\n";
}
std::string ActivityDefinition::getId() const { return activityId; }
std::string ActivityDefinition::getName() const { return name; }
std::string ActivityDefinition::getRequiredRoleId() const { return requiredRoleId; }