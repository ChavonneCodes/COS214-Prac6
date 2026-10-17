/*Class represents a role in the workflow blueprint
e.g. Manager Clerk. It holds the role's ID,name and a required permission level*/

#ifndef ROLE_DEFINITION_H
#define ROLE_DEFINITION_H

#include <string>

class RoleDefinition {
    private:
        std::string roleId;
        std::string name;
        int requiredLevel;//0=low,1=medium,2=high

    public:
        RoleDefinition(const std::string& roleId, const std::string& name,int requiredLevel);
        std::string getRoleId() const;
        std::string getName()  const;
        int getRequiredLevel() const;
};

#endif//ROLE_DEFINITION_H
 