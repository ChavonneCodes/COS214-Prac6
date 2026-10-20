#include "ConditionalRouting.h"
#include "RoutingHelper.h"

ConditionalRouting::ConditionalRouting(const std::string& condition,
                                       const std::string& trueTarget,
                                       const std::string& falseTarget)
    : condition(condition), trueTarget(trueTarget), falseTarget(falseTarget) {}

std::vector<const ActivityDefinition*> ConditionalRouting::next(
    const ActivityDefinition& /*completed*/,
    const WorkflowDefinition& definition,
    const std::string& outcome) {
    std::vector<const ActivityDefinition*> result;
    const ActivityDefinition* target =
        findActivity(definition, outcome == condition ? trueTarget : falseTarget);
    if (target) result.push_back(target);
    return result;
}

std::string ConditionalRouting::getName() const {
    return "Conditional(" + condition + ")";
}
