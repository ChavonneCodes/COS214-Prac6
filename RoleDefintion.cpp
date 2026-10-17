#include "RoleDefinition.h"

RoleDefinition::RoleDefinition(const std::string& roleId,const std::string& name,int requiredLevel):
roleId(roleId),name(name),requiredLevel(requiredLevel){}

std::string RoleDefinition::getRoleId() const{
    return roleId;
}

std::string RoleDefinition::getName() const{
    return name;
}

int RoleDefinition::getRequiredLevel() const{
    return requiredLevel;
}