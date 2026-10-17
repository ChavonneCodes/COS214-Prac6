/*Builder pattern-the product
This is the blueprint. It is immutable once created even though the builder
populates it before handing it over. It holds vectors of all the definition components*/
#ifndef WORKFLOW_DEFINITION_H
#define WORKFLOW_DEFINITION_H

#include <string>
#include <vector>
#include "ActivityDefinition.h"
#include "RoleDefinition.h"
#include "ApprovalChainDefinition.h"
#include "TransitionDefinition.h"

class WorkflowDefinition {
    private:
    std::string definitionId;
    std::string name;
    int version;
    std::string chainId;

    std::vector<ActivityDefinition> activities;
    std::vector<RoleDefinition> roles;
    std::vector<ApprovalChainDefinition> approvalChains;
    std::vector<TransitionDefinition> transitions;

    //root of Composite tree of stages and activities
    WorkflowComponent* root=nullptr; 

    public:
    WorkflowDefinition(const std::string& id,const std::string& name,int version);
     ~WorkflowDefinition(); 

    void setRoot(WorkflowComponent* rootNode);
    WorkflowComponent* getRoot() const;

    /*Methods used by the builder to populate the definition*/
    void addActivity(const ActivityDefinition& activity);
    void addRole(const RoleDefinition& role);
    void addApprovalChain(const ApprovalChainDefinition& approvalChain);
    void addTransition(const TransitionDefinition& transition);
    //Acccessors
    std::string getDefinitionId() const;
    std::string getName() const;
    int getVersion() const;
    const std::vector<ActivityDefinition>& getActivities() const;
    const std::vector<RoleDefinition>& getRoles() const;
    const std::vector<ApprovalChainDefinition>& getApprovalChains() const;
    const std::vector<TransitionDefinition>& getTransitions() const;
    ApprovalChainDefinition* getApprovalChainById(const std::string& chainId);

        WorkflowDefinition(const WorkflowDefinition&) = delete;
    WorkflowDefinition& operator=(const WorkflowDefinition&) = delete;
    WorkflowDefinition(WorkflowDefinition&& other) noexcept;
    WorkflowDefinition& operator=(WorkflowDefinition&& other) noexcept;
    
};

#endif//WORKFLOW_DEFINITION_H