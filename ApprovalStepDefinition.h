/*Represents a single step in a multi-level approval chain. It defines who
approves at this stppe, how long they have to do it (timeout), and what happens if
they dont(onTimeout)*/
#ifndef APPROVAL_STEP_DEFINITION_H
#define APPROVAL_STEP_DEFINITION_H

#include <string>

class ApprovalStepDefinition {
private:
    std::string roleId;
    int timeoutSeconds;
    std::string onTimeout; // "escalate", "auto-approve", "auto-reject"

public:
    ApprovalStepDefinition(const std::string& roleId, int timeoutSeconds, const std::string& onTimeout);
    
    std::string getRoleId() const;
    int getTimeout() const;
    std::string getOnTimeout() const;
};

#endif