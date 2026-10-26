#ifndef TIMEOUTESCALATIONHANDLER_H
#define TIMEOUTESCALATIONHANDLER_H

#include "ApprovalHandler.h"

class TimeoutEscalationHandler : public ApprovalHandler {
public:
    std::string getName() const override;

protected:
    HandlerResult handleRequest(Request& request) override;
};

#endif