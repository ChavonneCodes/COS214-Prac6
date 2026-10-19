/**
 * @file RoutingHelper.h
 * @brief Small lookup helper shared by the routing strategies.
 */
#ifndef ROUTING_HELPER_H
#define ROUTING_HELPER_H

#include <string>
#include "WorkflowDefinition.h"

/**
 * @brief Finds an activity in a workflow definition by id.
 * @param definition The workflow definition to search.
 * @param id         Id of the activity to find.
 * @return Pointer to the activity, or nullptr if it does not exist.
 */
inline const ActivityDefinition* findActivity(const WorkflowDefinition& definition,
                                              const std::string& id) {
    const std::vector<ActivityDefinition>& all = definition.getActivities();
    for (size_t i = 0; i < all.size(); ++i) {
        if (all[i].getId() == id) return &all[i];
    }
    return nullptr;
}

#endif // ROUTING_HELPER_H
