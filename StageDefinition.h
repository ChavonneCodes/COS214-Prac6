#ifndef STAGE_DEFINITION_H
#define STAGE_DEFINITION_H

#include "WorkflowComponent.h"
#include <iostream>

class StageDefinition : public WorkflowComponent {
private:
    std::string stageId;
    std::string name;
    std::vector<WorkflowComponent*> children; // Composites own their children

public:
    StageDefinition(const std::string& id, const std::string& name);
    ~StageDefinition() override; // Must clean up children

    std::string getId() const override;
    std::string getName() const override;

    void addChild(WorkflowComponent* child) override;
    std::vector<WorkflowComponent*> getChildren() const override;
    void print(int indentLevel = 0) const override;
};

#endif