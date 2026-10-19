#ifndef INSTANCE_FACTORY_H
#define INSTANCE_FACTORY_H

#include <string>
#include "WorkflowComponent.h"
#include "RuntimeComponent.h"
#include "AssignmentStrategy.h"
#include "Participant.h"
#include <vector>


class InstanceFactory {
public:
    // Takes a definition node and creates the matching runtime tree,accepts the strategy and candidates as well
    static RuntimeComponent* instantiateTree(const std::string& instanceId, WorkflowComponent* defNode,AssignmentStrategy* assignmentStrategy, const std::vector<Participant*>& candidates);

};

#endif