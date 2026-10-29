#include "SubProcessDefinition.h"

SubProcessDefinition::SubProcessDefinition(const std::string& id, const std::string& calledDefId, int version)
    : subProcessId(id), calledDefinitionId(calledDefId), calledVersion(version) {}

std::string SubProcessDefinition::getId() const { return subProcessId; }
std::string SubProcessDefinition::getName() const { return "Call SubProcess: " + calledDefinitionId; }
std::string SubProcessDefinition::getCalledDefinitionId() const { return calledDefinitionId; }
int SubProcessDefinition::getCalledVersion() const { return calledVersion; }

SubProcessDefinition* SubProcessDefinition::clone() const {
    return new SubProcessDefinition(*this); // only values (ids and a version number), so a copy is already deep
}

void SubProcessDefinition::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "* SubProcess: Calls " << calledDefinitionId << " (v" << calledVersion << ")\n";
}