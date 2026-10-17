#ifndef SUB_PROCESS_INSTANCE_H
#define SUB_PROCESS_INSTANCE_H

#include "RuntimeComponent.h"
#include "SubProcessDefinition.h"

class SubProcessInstance : public RuntimeComponent {
private:
    std::string instanceId;
    SubProcessDefinition* definition;
    std::string state;

public:
    SubProcessInstance(const std::string& instanceId, SubProcessDefinition* def);
    
    std::string getId() const override;
    std::string getState() const override;
    void setState(const std::string& newState) override;
    void print(int indentLevel = 0) const override;
};

#endif