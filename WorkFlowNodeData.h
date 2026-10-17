#ifndef WORKFLOW_NODE_DATA_H
#define WORKFLOW_NODE_DATA_H

#include <string>
#include <vector>

enum class NodeType { STAGE, ACTIVITY, SUBPROCESS };

struct WorkflowNodeData {
    NodeType type;
    std::string id;
    std::string name;
    
    // Used if type == ACTIVITY
    std::string roleId;
    
    // Used if type == SUBPROCESS
    std::string calledDefId;
    int calledVersion = 1;
    
    // Used if type == STAGE
    std::vector<WorkflowNodeData> children;
};

#endif