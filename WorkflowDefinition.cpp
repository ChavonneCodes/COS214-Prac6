#include "WorkflowDefinition.h"
#include "WorkflowComponent.h"
#include <utility>
#include <memory>

WorkflowDefinition::WorkflowDefinition(const std::string& id,const std::string& name,int version):
definitionId(id),name(name),version(version),root(nullptr) {}

WorkflowDefinition::~WorkflowDefinition() {
    if (root != nullptr) {
        delete root; // Recursively deletes the whole tree
    }
}

void WorkflowDefinition::setRoot(WorkflowComponent* rootNode) {
    root = rootNode;
}

WorkflowComponent* WorkflowDefinition::getRoot() const {
    return root;
}

WorkflowDefinition* WorkflowDefinition::cloneWithVersion(int newVersion) const {
    if (newVersion == version) {
        throw std::invalid_argument("cloneWithVersion: the copy needs a different version number");
    }

    std::unique_ptr<WorkflowDefinition> copy(new WorkflowDefinition(definitionId, name, newVersion));
    copy->chainId = chainId;
    // These lists hold plain values, so copying the vectors is already a deep copy.
    copy->activities = activities;
    copy->roles = roles;
    copy->approvalChains = approvalChains;
    copy->transitions = transitions;
    if (root != nullptr) {
        copy->root = root->clone(); // the Prototype step: duplicate the whole Composite tree
    }
    return copy.release();
}

void WorkflowDefinition::addActivity(const ActivityDefinition& activity){
    activities.push_back(activity);
}

void WorkflowDefinition::addRole(const RoleDefinition& role){
    roles.push_back(role);
}

void WorkflowDefinition::addApprovalChain(const ApprovalChainDefinition& approvalChain){
    approvalChains.push_back(approvalChain);
}

void WorkflowDefinition::addTransition(const TransitionDefinition& transition){
    transitions.push_back(transition);
}

std::string WorkflowDefinition::getDefinitionId() const{
    return definitionId;
}

std::string WorkflowDefinition::getName() const{
    return name;
}

int WorkflowDefinition::getVersion() const{
    return version;
}

const std::vector<ActivityDefinition>& WorkflowDefinition::getActivities() const{
    return activities;
}

const std::vector<RoleDefinition>& WorkflowDefinition::getRoles() const{
    return roles;
}

const std::vector<ApprovalChainDefinition>& WorkflowDefinition::getApprovalChains() const{
    return approvalChains;
}

const std::vector<TransitionDefinition>& WorkflowDefinition::getTransitions() const{
    return transitions;
}

ApprovalChainDefinition* WorkflowDefinition::getApprovalChainById(const std::string& chainId) {
    for (auto& chain : approvalChains) {
        if (chain.getChainId() == chainId) {
            return &chain;
        }
    }
    return nullptr; 
}



// Implement Move Constructor
WorkflowDefinition::WorkflowDefinition(WorkflowDefinition&& other) noexcept
    : definitionId(std::move(other.definitionId)),
      name(std::move(other.name)),
      version(other.version),
      chainId(std::move(other.chainId)),
      activities(std::move(other.activities)),
      roles(std::move(other.roles)),
      approvalChains(std::move(other.approvalChains)),
      transitions(std::move(other.transitions)),
      root(other.root) 
{
    other.root = nullptr; // Vital: prevent the original object from deleting the tree
}

// Implement Move Assignment
WorkflowDefinition& WorkflowDefinition::operator=(WorkflowDefinition&& other) noexcept {
    if (this != &other) {
        delete root; // Free existing memory
        
        definitionId = std::move(other.definitionId);
        name = std::move(other.name);
        version = other.version;
        chainId = std::move(other.chainId);
        activities = std::move(other.activities);
        roles = std::move(other.roles);
        approvalChains = std::move(other.approvalChains);
        transitions = std::move(other.transitions);
        root = other.root;
        
        other.root = nullptr;
    }
    return *this;
}