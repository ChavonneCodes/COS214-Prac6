/*
Holds an ordered list of Approva;Deffinition objects. Represents the entire  approval process for a specific activity*/
#ifndef APPROVAL_CHAIN_DEFINITION_H
#define APPROVAL_CHAIN_DEFINITION_H

#include <string>
#include <vector>
#include "ApprovalStepDefinition.h"

class ApprovalChainDefinition {
private:
    std::string chainId;
    std::vector<ApprovalStepDefinition> steps;

public:
    ApprovalChainDefinition(const std::string& chainId);
    void addStep(const ApprovalStepDefinition& step);
    
    std::string getChainId() const;
    const std::vector<ApprovalStepDefinition>& getSteps() const;
};

#endif