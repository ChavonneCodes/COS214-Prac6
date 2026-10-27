#include "SubProcessInstance.h"
#include "RuntimeVisitor.h"

SubProcessInstance::SubProcessInstance(const std::string& instanceId, SubProcessDefinition* def)
    : instanceId(instanceId), definition(def), state("IDLE") {}

std::string SubProcessInstance::getId() const { return definition->getId(); }
std::string SubProcessInstance::getState() const { return state; }
void SubProcessInstance::setState(const std::string& newState) { state = newState; }

void SubProcessInstance::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "[SubProcessInstance] Calls " << definition->getCalledDefinitionId() 
              << " (State: " << state << ")\n";
}

void SubProcessInstance::accept(RuntimeVisitor& visitor) { visitor.visitSubProcess(*this); }