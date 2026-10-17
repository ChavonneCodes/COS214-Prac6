#ifndef WORKFLOW_DIRECTOR_H
#define WORKFLOW_DIRECTOR_H

#include "WorkflowBuilder.h"
#include "WorkflowBlueprint.h"

class WorkflowDirector {
public:
    void construct(WorkflowBuilder& builder, WorkflowBlueprint& blueprint);
};

#endif