#ifndef STAGE_INSTANCE_H
#define STAGE_INSTANCE_H

#include "RuntimeComponent.h"
#include "StageDefinition.h"

class StageInstance : public RuntimeComponent {
private:
    std::string instanceId;
    StageDefinition* definition;
    std::string state;
    std::vector<RuntimeComponent*> children;

public:
    StageInstance(const std::string& instanceId, StageDefinition* def);
    ~StageInstance() override; // Must delete children

    std::string getId() const override;
    std::string getState() const override;
    void setState(const std::string& newState) override;
    void print(int indentLevel = 0) const override;

    void addChild(RuntimeComponent* child) override;
    std::vector<RuntimeComponent*> getChildren() const override;
};

#endif