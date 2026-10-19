#ifndef WORK_ITEM_H
#define WORK_ITEM_H

#include "RuntimeComponent.h"
#include "Participant.h"   // Strategy: the person an AssignmentStrategy picks
#include "ActivityDefinition.h"

class WorkItem : public RuntimeComponent {
private:
    std::string instanceId;
    ActivityDefinition* definition;
    Participant* assignedParticipant = nullptr;  ///< Who the item is assigned to (not owned, may be null).
    std::string state;

public:
    WorkItem(const std::string& instanceId, ActivityDefinition* def);

    /**
     * @brief Returns the id of the role needed to perform this work item (used by AssignmentStrategy).
     * @return The required role id.
     */
    std::string getRequiredRoleId() const;

    /**
     * @brief Returns the participant this item is assigned to.
     * @return The assigned participant, or nullptr if unassigned.
     */
    Participant* getAssignedParticipant() const;

    /**
     * @brief Assigns the item to a participant and sets its state to "ASSIGNED".
     *
     * If the item was already assigned, the previous participant loses one
     * open item and the new participant gains one.
     * @param participant Who to assign the item to (not owned).
     * @return true on success, false (and no change) if participant is nullptr.
     */
    bool assignTo(Participant* participant);
    
    std::string getId() const override;
    std::string getState() const override;
    void setState(const std::string& newState) override;
    void print(int indentLevel = 0) const override;
};

#endif