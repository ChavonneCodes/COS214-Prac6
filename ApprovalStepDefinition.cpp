#include "ApprovalStepDefinition.h"

ApprovalStepDefinition::ApprovalStepDefinition(const std::string& roleId, int timeoutSeconds, const std::string& onTimeout)
    : roleId(roleId), timeoutSeconds(timeoutSeconds), onTimeout(onTimeout) {}

std::string ApprovalStepDefinition::getRoleId() const { return roleId; }
int ApprovalStepDefinition::getTimeout() const { return timeoutSeconds; }
std::string ApprovalStepDefinition::getOnTimeout() const { return onTimeout; }