
/**
 * @file Participant.h
 * @brief A person (or system) who can perform work items.
 *
 * Used by the **Strategy** pattern: AssignmentStrategy classes choose among
 * participants.
 */
#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string>
#include <vector>

/**
 * @class Participant
 * @brief Someone who can be assigned work, with roles, a seniority level and a workload.
 */
class Participant {
private:
    std::string participantId;          ///< Unique id.
    std::string name;                   ///< Display name.
    std::vector<std::string> roleIds;   ///< Ids of the RoleDefinitions this participant holds.
    int level;                          ///< Seniority, comparable with RoleDefinition::getRequiredLevel().
    int openItems;                      ///< Work items currently assigned (never negative).

public:
    /**
     * @brief Creates a participant with no open work items.
     * @param participantId Unique id.
     * @param name          Display name.
     * @param roleIds       Ids of the roles this participant holds.
     * @param level         Seniority level (default 0).
     */
    Participant(const std::string& participantId, const std::string& name,
                const std::vector<std::string>& roleIds, int level = 0);

    /**

     * @brief Returns the unique id.

     * @return The unique id.

     */
    std::string getParticipantId() const;
    /**
     * @brief Returns the display name.
     * @return The display name.
     */
    std::string getName() const;
    /**
     * @brief Returns the seniority level.
     * @return The seniority level.
     */
    int getLevel() const;

    /**
     * @brief Checks whether this participant holds a role.
     * @param roleId Id of the role to look for.
     * @return true if the participant holds that role.
     */
    bool hasRole(const std::string& roleId) const;

    /**

     * @brief Returns how many work items are currently assigned.

     * @return How many work items are currently assigned.

     */
    int getOpenItems() const;
    /** @brief Records that one more work item was assigned. */
    void addOpenItem();
    /** @brief Records that one work item was finished or reassigned (stops at zero). */
    void closeOpenItem();
};

#endif // PARTICIPANT_H

