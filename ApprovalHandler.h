#ifndef APPROVALHANDLER_H
#define APPROVALHANDLER_H

#include "Request.h"
#include <string>

enum class HandlerResult{
    Continue,
    Handled,
    Rejected
};

class ApprovalHandler {
public:
    virtual ~ApprovalHandler() = default;

    virtual std::string getName() const;

    void setNext(ApprovalHandler* handler);

    HandlerResult authorise(Request& request);

protected:
    // Each derived handler defines its own approval logic.
    virtual HandlerResult handleRequest(Request& request);

private:
    ApprovalHandler* next = nullptr;
};

#endif
