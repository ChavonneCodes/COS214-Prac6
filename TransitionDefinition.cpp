#include "TransitionDefinition.h"

TransitionDefinition::TransitionDefinition(const std::string& from,const std::string& to,const std::string&  condition):
fromActivityId(from),toActivityId(to),condition(condition){}

std::string TransitionDefinition::getFrom() const{
    return fromActivityId;
}

std::string TransitionDefinition::getTo() const{
    return toActivityId;
}

std::string TransitionDefinition::getCondition() const{
    return condition;
}