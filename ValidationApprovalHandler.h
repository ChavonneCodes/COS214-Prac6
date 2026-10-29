#ifndef VALIDATIONAPPROVALHANDLER_H
#define VALIDATIONAPPROVALHANDLER_H

#include "ApprovalHandler.h"

class ValidationApprovalHandler : public ApprovalHandler {
public:
    std::string getName() const override;

protected:
    HandlerResult handleRequest(Request& request) override;
};

#endif