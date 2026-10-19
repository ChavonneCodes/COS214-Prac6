#include "InstanceFactory.h"
#include "ActivityDefinition.h"
#include "StageDefinition.h"
#include "SubProcessDefinition.h"
#include "WorkItem.h"
#include "StageInstance.h"
#include "SubProcessInstance.h"
#include <iostream>

RuntimeComponent* InstanceFactory::instantiateTree(const std::string& instanceId, WorkflowComponent* defNode, AssignmentStrategy* assignmentStrategy, const std::vector<Participant*>& candidates) {
    if (!defNode) return nullptr;

    // 1. Is it an Activity?
    ActivityDefinition* actDef = dynamic_cast<ActivityDefinition*>(defNode);
    if (actDef) {
        WorkItem* workItem = new WorkItem(instanceId, actDef);

        // --- STRATEGY PATTERN IN ACTION ---
        // Only do this if we actually created a WorkItem!
        if (assignmentStrategy) {
            Participant* assigned = assignmentStrategy->assign(*workItem, candidates);
            if (assigned) {
                workItem->assignTo(assigned);
                std::cout << "  [Factory] Assigned " << actDef->getName() 
                          << " to " << assigned->getName() 
                          << " using strategy " << assignmentStrategy->getName() << "\n";
            } else {
                std::cout << "  [Factory] Warning: Could not find a candidate for " << actDef->getName() << "\n";
            }
        }
        return workItem; // Return the fully assigned WorkItem
    }

    // 2. Is it a Stage?
    StageDefinition* stageDef = dynamic_cast<StageDefinition*>(defNode);
    if (stageDef) {
        StageInstance* stageInstance = new StageInstance(instanceId, stageDef);
        for (WorkflowComponent* childDef : stageDef->getChildren()) {
            // Pass strategy down the tree recursively
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