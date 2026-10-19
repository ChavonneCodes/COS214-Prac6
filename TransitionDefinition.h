/*Represented a  connection(edge) between two activities
It is not a node in the tree; it is the rule that says after Activity A go to Activity B*/
#ifndef TRANSITION_DEFINITION_H
#define TRANSITION_DEFINITION_H

#include <string>

class TransitionDefinition{
    private:
        std::string fromActivityId;
        std::string toActivityId;
        std::string condition; //eg "always","passed","failed"

    public:
        TransitionDefinition(const std::string& from,const std::string& to,const std::string& condition);
        std::string getFrom() const;
        std::string getTo() const;
        std::string getCondition() const;
};

#endif