#ifndef WORKFLOW_BUILDER_H
#define WORKFLOW_BUILDER_H

#include "WorkflowDefinition.h"
#include "WorkFlowNodeData.h"
#include <string>

class WorkflowBuilder {
public:
    virtual ~WorkflowBuilder() = default;

    // Setup methods (User provides the data)
    virtual WorkflowBuilder& setId(const std::string& id) = 0;
    virtual WorkflowBuilder& setName(const std::string& name) = 0;
    virtual WorkflowBuilder& setVersion(int version) = 0;

    // Add methods (User chooses exactly what to add)
    virtual WorkflowBuilder& addRole(const std::string& roleId, const std::string& roleName, int level) = 0;
    virtual WorkflowBuilder& addActivity(const std::string& actId, const std::string& name, const std::string& roleId) = 0;
    virtual WorkflowBuilder& addTransition(const std::string& from, const std::string& to, const std::string& condition) = 0;
    virtual WorkflowBuilder& addApprovalChain(const ApprovalChainDefinition& chain) = 0;

    // Finalize and return the product
    virtual WorkflowDefinition build() = 0;

    virtual WorkflowBuilder& setRoot(const WorkflowNodeData& rootData) = 0;
};

#endif