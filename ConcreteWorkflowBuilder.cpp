#include "ConcreteWorkflowBuilder.h"
#include "StageDefinition.h"
#include "SubProcessDefinition.h"
#include <stdexcept>

ConcreteWorkflowBuilder::ConcreteWorkflowBuilder() : version(1) {}

WorkflowBuilder& ConcreteWorkflowBuilder::setId(const std::string& id) {
    definitionId = id;
    return *this;
}

WorkflowBuilder& ConcreteWorkflowBuilder::setName(const std::string& n) {
    name = n;
    return *this;
}

WorkflowBuilder& ConcreteWorkflowBuilder::setVersion(int v) {
    version = v;
    return *this;
}

WorkflowBuilder& ConcreteWorkflowBuilder::addRole(const std::string& roleId, const std::string& roleName, int level) {
    roles.push_back(RoleDefinition(roleId, roleName, level));
    return *this;
}

WorkflowBuilder& ConcreteWorkflowBuilder::addActivity(const std::string& actId, const std::string& actName, const std::string& roleId) {
    activities.push_back(ActivityDefinition(actId, actName, roleId));
    return *this;
}

WorkflowBuilder& ConcreteWorkflowBuilder::addTransition(const std::string& from, const std::string& to, const std::string& condition) {
    transitions.push_back(TransitionDefinition(from, to, condition));
    return *this;
}

WorkflowBuilder& ConcreteWorkflowBuilder::addApprovalChain(const ApprovalChainDefinition& chain) {
    approvalChains.push_back(chain);
    return *this;
}

WorkflowBuilder& ConcreteWorkflowBuilder::setRoot(const WorkflowNodeData& rootData) {
    rootTreeData = rootData;
    hasRoot = true;
    return *this;
}

// The recursive builder function!
WorkflowComponent* ConcreteWorkflowBuilder::createComponent(const WorkflowNodeData& data) {
    if (data.type == NodeType::STAGE) {
        StageDefinition* stage = new StageDefinition(data.id, data.name);
        // Recursively build children
        for (const auto& childData : data.children) {
            stage->addChild(createComponent(childData));
        }
        return stage;
    } 
    else if (data.type == NodeType::ACTIVITY) {
        return new ActivityDefinition(data.id, data.name, data.roleId);
    } 
    else if (data.type == NodeType::SUBPROCESS) {
        return new SubProcessDefinition(data.id, data.calledDefId, data.calledVersion);
    }
    return nullptr;
}

// Update build():
WorkflowDefinition ConcreteWorkflowBuilder::build() {
    if (definitionId.empty()) throw std::runtime_error("definitionId is required");
    if (name.empty()) throw std::runtime_error("name is required");

    WorkflowDefinition def(definitionId, name, version);
    
    // Add flat data
    for (const auto& r : roles) def.addRole(r);
    for (const auto& a : activities) def.addActivity(a);
    for (const auto& t : transitions) def.addTransition(t);
    for (const auto& c : approvalChains) def.addApprovalChain(c);

    // If the user provided a tree structure, the Builder constructs it NOW
    if (hasRoot) {
        WorkflowComponent* rootComponent = createComponent(rootTreeData);
        def.setRoot(rootComponent);
    }

    return def;
}