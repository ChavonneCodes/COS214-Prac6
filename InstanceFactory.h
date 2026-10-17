#ifndef INSTANCE_FACTORY_H
#define INSTANCE_FACTORY_H

#include <string>
#include "WorkflowComponent.h"
#include "RuntimeComponent.h"

class InstanceFactory {
public:
    // Takes a definition node and creates the matching runtime tree
    static RuntimeComponent* instantiateTree(const std::string& instanceId, WorkflowComponent* defNode);
};

#endif