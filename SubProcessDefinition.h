#ifndef SUB_PROCESS_DEFINITION_H
#define SUB_PROCESS_DEFINITION_H

#include "WorkflowComponent.h"
#include <iostream>

class SubProcessDefinition : public WorkflowComponent {
private:
    std::string subProcessId;
    std::string calledDefinitionId;
    int calledVersion;

public:
    SubProcessDefinition(const std::string& id, const std::string& calledDefId, int version);
    
    std::string getId() const override;
    std::string getName() const override;
    std::string getCalledDefinitionId() const;
    int getCalledVersion() const;
    void print(int indentLevel = 0) const override;
};

#endif