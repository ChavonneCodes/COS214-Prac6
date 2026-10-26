#include "InstanceFactory.h"
#include "ActivityDefinition.h"
#include "StageDefinition.h"
#include "SubProcessDefinition.h"
#include "WorkItem.h"
#include "StageInstance.h"
#include "SubProcessInstance.h"
#include <iostream>

RuntimeComponent* InstanceFactory::instantiateTree(
    const std::string& instanceId, 
    WorkflowComponent* defNode, 
    AssignmentStrategy* assignmentStrategy, 
    const std::vector<Participant*>& candidates) 
{
    if (!defNode) return nullptr;

    // 1. Is it an Activity?
    ActivityDefinition* actDef = dynamic_cast<ActivityDefinition*>(defNode);
    if (actDef) {
        WorkItem* workItem = new WorkItem(instanceId, actDef);

        if (assignmentStrategy) {
            Participant* assigned = assignmentStrategy->assign(*workItem, candidates);
            if (assigned) {
                workItem->assignTo(assigned);
                std::cout << "  [Factory] Assigned " << actDef->getName() 
                          << " to " << assigned->getName() << "\n";
            }
        }
        return workItem; 
    }

    // 2. Is it a Stage?
    StageDefinition* stageDef = dynamic_cast<StageDefinition*>(defNode);
    if (stageDef) {
        StageInstance* stageInstance = new StageInstance(instanceId, stageDef);
        // Recursively build all children
        for (WorkflowComponent* childDef : stageDef->getChildren()) {
            RuntimeComponent* childInstance = instantiateTree(instanceId, childDef, assignmentStrategy, candidates);
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