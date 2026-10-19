#include "SequentialRouting.h"
#include "RoutingHelper.h"

std::vector<const ActivityDefinition*> SequentialRouting::next(
    const ActivityDefinition& completed,
    const WorkflowDefinition& definition,
    const std::string& /*outcome*/) {
    std::vector<const ActivityDefinition*> result;
    const std::vector<TransitionDefinition>& transitions = definition.getTransitions();
    for (size_t i = 0; i < transitions.size(); ++i) {
        if (transitions[i].getFrom() == completed.getId() &&
            transitions[i].getCondition() == "always") {
            const ActivityDefinition* target = findActivity(definition, transitions[i].getTo());
            if (target) result.push_back(target);
        }
    }
    return result;
}

std::string SequentialRouting::getName() const {
    return "Sequential";
}
