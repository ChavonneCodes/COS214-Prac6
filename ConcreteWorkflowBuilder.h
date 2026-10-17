#ifndef CONCRETE_WORKFLOW_BUILDER_H
#define CONCRETE_WORKFLOW_BUILDER_H

#include "WorkflowBuilder.h"
#include  "WorkFlowNodeData.h"
#include <vector>

class ConcreteWorkflowBuilder : public WorkflowBuilder {
private:
    std::string definitionId;
    std::string name;
    int version;
    
    std::vector<ActivityDefinition> activities;
    std::vector<TransitionDefinition> transitions;
    std::vector<RoleDefinition> roles;
    std::vector<ApprovalChainDefinition> approvalChains;
    WorkflowNodeData rootTreeData;
    bool hasRoot = false;

    // Recursive helper to build the Composite tree
    WorkflowComponent* createComponent(const WorkflowNodeData& data);

public:
    ConcreteWorkflowBuilder();

    // Implementing the Abstract Builder steps
    WorkflowBuilder& setId(const std::string& id) override;
    WorkflowBuilder& setName(const std::string& name) override;
    WorkflowBuilder& setVersion(int version) override;

    WorkflowBuilder& addRole(const std::string& roleId, const std::string& roleName, int level) override;
    WorkflowBuilder& addActivity(const std::string& actId, const std::string& name, const std::string& roleId) override;
    WorkflowBuilder& addTransition(const std::string& from, const std::string& to, const std::string& condition) override;
    WorkflowBuilder& addApprovalChain(const ApprovalChainDefinition& chain) override;

    WorkflowDefinition build() override;

    WorkflowBuilder& setRoot(const WorkflowNodeData& rootData) override;
};

#endif