/**
 * @file RoleDefinition.h
 * @brief Defines the RoleDefinition class.
 * 
 * Represents a role in the workflow blueprint (e.g., Manager, Clerk). 
 * It holds the role's ID, name, and a required permission level.
 */

#ifndef ROLE_DEFINITION_H
#define ROLE_DEFINITION_H

#include <string>

/**
 * @class RoleDefinition
 * @brief Represents a specific role within the workflow system.
 * 
 * This class is used to define who is allowed to perform certain activities.
 * It is referenced by ActivityDefinition and used by AssignmentStrategies
 * to determine which Participant should be assigned to a WorkItem.
 */
class RoleDefinition {
    private:
        std::string roleId;      ///< Unique identifier for the role (e.g., "manager").
        std::string name;        ///< Human-readable name of the role (e.g., "Manager").
        int requiredLevel;       ///< The permission level required (0=low, 1=medium, 2=high).

    public:
        /**
         * @brief Constructs a new RoleDefinition.
         * 
         * @param roleId The unique identifier for the role.
         * @param name The human-readable name of the role.
         * @param requiredLevel The permission level (0=low, 1=medium, 2=high).
         */
        RoleDefinition(const std::string& roleId, const std::string& name, int requiredLevel);
        
        /**
         * @brief Retrieves the unique identifier of the role.
         * 
         * @return The role ID as a string.
         */
        std::string getRoleId() const;
        
        /**
         * @brief Retrieves the human-readable name of the role.
         * 
         * @return The role name as a string.
         */
        std::string getName() const;
        
        /**
         * @brief Retrieves the required permission level for this role.
         * 
         * @return The permission level (0=low, 1=medium, 2=high).
         */
        int getRequiredLevel() const;
};

#endif // ROLE_DEFINITION_H