#include "WorkflowDirector.h"

void WorkflowDirector::construct(WorkflowBuilder& builder, WorkflowBlueprint& blueprint) {
    builder.setId(blueprint.id);
    builder.setName(blueprint.name);
    builder.setVersion(blueprint.version);

    for (const auto& role : blueprint.roles) {
        builder.addRole(role.getRoleId(), role.getName(), role.getRequiredLevel());
    }
    for (const auto& activity : blueprint.activities) {
        builder.addActivity(activity.getId(), activity.getName(), activity.getRequiredRoleId());
    }
    for (const auto& transition : blueprint.transitions) {
        builder.addTransition(transition.getFrom(), transition.getTo(), transition.getCondition());
    }
    for (const auto& chain : blueprint.approvalChains) {
        builder.addApprovalChain(chain);
    }

    // Pass the raw data to the Builder
    if (blueprint.hasRootTree) {
        builder.setRoot(blueprint.rootTreeData);
    }
}