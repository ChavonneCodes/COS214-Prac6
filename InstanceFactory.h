/**
 * @file InstanceFactory.h
 * @brief Defines the InstanceFactory class.
 * 
 * Responsible for instantiating runtime components from static blueprint nodes.
 */

#ifndef INSTANCE_FACTORY_H
#define INSTANCE_FACTORY_H

#include "RuntimeComponent.h"
#include "WorkflowComponent.h"
#include "AssignmentStrategy.h"
#include "Participant.h"
#include <string>
#include <vector>

/**
 * @class InstanceFactory
 * @brief A Simple Factory that creates runtime nodes from definition nodes.
 * 
 * This class bridges the gap between the static Definition tree (Composite)
 * and the live Runtime tree. It uses dynamic type checking to determine whether
 * a given WorkflowComponent is an Activity, Stage, or SubProcess, and instantiates
 * the corresponding RuntimeComponent.
 * 
 * It supports eager instantiation: it recursively builds the entire tree at once.
 */
class InstanceFactory {
public:
    /**
     * @brief Recursively creates the entire runtime tree from a definition tree.
     * 
     * @param instanceId The unique ID of the overarching workflow instance.
     * @param defNode Pointer to the root static definition node.
     * @param assignmentStrategy Pointer to the strategy used to assign work (may be nullptr).
     * @param candidates The pool of available participants to choose from.
     * @return A newly allocated RuntimeComponent tree (caller assumes ownership).
     */
    static RuntimeComponent* instantiateTree(
        const std::string& instanceId, 
        WorkflowComponent* defNode, 
        AssignmentStrategy* assignmentStrategy, 
        const std::vector<Participant*>& candidates
    );
};

#endif // INSTANCE_FACTORY_H