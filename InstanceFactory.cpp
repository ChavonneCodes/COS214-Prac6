#include "InstanceFactory.h"
#include "ActivityDefinition.h"
#include "StageDefinition.h"
#include "SubProcessDefinition.h"
#include "WorkItem.h"
#include "StageInstance.h"
#include "SubProcessInstance.h"

RuntimeComponent* InstanceFactory::instantiateTree(const std::string& instanceId, WorkflowComponent* defNode) {
    if (!defNode) return nullptr;

    // 1. Is it an Activity?
    ActivityDefinition* actDef = dynamic_cast<ActivityDefinition*>(defNode);
    if (actDef) {
        return new WorkItem(instanceId, actDef);
    }

    // 2. Is it a Stage?
    StageDefinition* stageDef = dynamic_cast<StageDefinition*>(defNode);
    if (stageDef) {
        StageInstance* stageInstance = new StageInstance(instanceId, stageDef);
        for (WorkflowComponent* childDef : stageDef->getChildren()) {
            RuntimeComponent* childInstance = instantiateTree(instanceId, childDef);
            if (childInstance) {
                stageInstance->addChild(childInstance);
            }
        }
        return stageInstance;
    }

    // 3. Is it a SubProcess?
    SubProcessDefinition* subDef = dynamic_cast<SubProcessDefinition*>(defNode);
    if (subDef) {
        return new SubProcessInstance(instanceId, subDef);
    }

    return nullptr;
}