/**
 * @file WorkflowNodeData.h
 * @brief Defines the raw data structures used to build the Composite tree.
 * 
 * This file provides a Data Transfer Object (DTO) that allows users to define
 * nested workflow structures (Stages, Activities, SubProcesses) without needing 
 * to manually allocate C++ pointers. The Builder reads this data to instantiate 
 * the actual Composite objects.
 */

#ifndef WORKFLOW_NODE_DATA_H
#define WORKFLOW_NODE_DATA_H

#include <string>
#include <vector>

/**
 * @enum NodeType
 * @brief Defines the type of a workflow node.
 * 
 * Used to determine which fields in the WorkflowNodeData struct are applicable.
 */
enum class NodeType { 
    STAGE,      ///< A composite node that groups other nodes.
    ACTIVITY,   ///< A leaf node representing a single piece of work.
    SUBPROCESS  ///< A leaf node that calls an entirely separate workflow definition.
};

/**
 * @struct WorkflowNodeData
 * @brief A raw data representation of a single node in the workflow hierarchy.
 * 
 * This struct is designed to be populated by the user (e.g., via a TUI or JSON file)
 * and passed to the ConcreteWorkflowBuilder. The Builder recursively converts 
 * this raw data into instantiated StageDefinition, ActivityDefinition, and 
 * SubProcessDefinition objects.
 */
struct WorkflowNodeData {
    NodeType type;          ///< The type of node (determines which fields below are used).
    std::string id;         ///< The unique identifier for the node.
    std::string name;       ///< The human-readable name of the node.
    
    // --- Conditional Fields ---
    
    std::string roleId;     ///< Used only if type == ACTIVITY. The ID of the required role.
    
    std::string calledDefId;///< Used only if type == SUBPROCESS. The ID of the workflow being called.
    int calledVersion = 1;  ///< Used only if type == SUBPROCESS. The version of the workflow being called.
    
    std::vector<WorkflowNodeData> children; ///< Used only if type == STAGE. The nested child nodes.
};

#endif // WORKFLOW_NODE_DATA_H