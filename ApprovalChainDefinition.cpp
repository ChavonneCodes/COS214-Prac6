#include "ApprovalChainDefinition.h"

ApprovalChainDefinition::ApprovalChainDefinition(const std::string& chainId)
    : chainId(chainId) {}

void ApprovalChainDefinition::addStep(const ApprovalStepDefinition& step) {
    steps.push_back(step);
}

std::string ApprovalChainDefinition::getChainId() const { return chainId; }
const std::vector<ApprovalStepDefinition>& ApprovalChainDefinition::getSteps() const { return steps; }