/*Composite pattern:Leaf node in the activity tree
Represents a single activity in the workflow
Holds the step's ID,name and ID of the role required to perform it*/
#ifndef ACTIVITY_DEFINITION_H
#define ACTIVITY_DEFINITION_H
#include "WorkflowComponent.h"
#include <iostream>
#include <string>

class ActivityDefinition :public WorkflowComponent {
private:
    std::string activityId;
    std::string name;
    std::string requiredRoleId;
    std::string approvalChainId; 

public:
    ActivityDefinition(const std::string& id, const std::string& name, 
                       const std::string& roleId, const std::string& chainId = "");

    std::string getApprovalChainId() const;
    void print(int indentLevel) const override;
    std::string getId() const override;
    std::string getName() const override;
    std::string getRequiredRoleId() const;
};


#endif//ACTIVITY_DEFINITION_H