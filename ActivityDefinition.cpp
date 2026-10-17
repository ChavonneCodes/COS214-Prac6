#include "ActivityDefinition.h"

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