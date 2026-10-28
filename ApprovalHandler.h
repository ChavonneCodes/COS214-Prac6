#ifndef APPROVALHANDLER_H
#define APPROVALHANDLER_H

#include "Request.h"
#include <string>

class ApprovalHandler {
public:
    virtual ~ApprovalHandler() = default;

    virtual std::string getName();

    void setNext(ApprovalHandler* handler);

    bool authorise(Request& request);

protected:
    // Each derived handler defines its own approval logic.
    virtual bool handleRequest(Request&);

private:
    ApprovalHandler* next = nullptr;
};

#endif
