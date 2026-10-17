#ifndef WORKFLOW_BLUEPRINT_H
#define WORKFLOW_BLUEPRINT_H

#include <string>
#include <vector>
#include "RoleDefinition.h"
#include "ActivityDefinition.h"
#include "TransitionDefinition.h"
#include "ApprovalChainDefinition.h"
#include "WorkflowComponent.h"
#include "WorkflowNodeData.h"

struct WorkflowBlueprint {
    std::string id;
    std::string name;
    int version = 1;
    
    std::vector<RoleDefinition> roles;
    std::vector<ActivityDefinition> activities;
    std::vector<TransitionDefinition> transitions;
    std::vector<ApprovalChainDefinition> approvalChains;
    
    // The Composite Tree (optional, if the user wants to build a nested structure)
    WorkflowNodeData rootTreeData; 
    bool hasRootTree = false;
};

#endif