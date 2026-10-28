#ifndef WORKFLOWSTATEAPPROVALHANDLER_H
#define WORKFLOWSTATEAPPROVALHANDLER_H

#include "ApprovalHandler.h"

class WorkflowStateApprovalHandler : public ApprovalHandler {
public:
    std::string getName() const override;

protected:
    HandlerResult handleRequest(Request& request) override;
};

#endif