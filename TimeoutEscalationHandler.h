#ifndef TIMEOUTESCALATIONHANDLER_H
#define TIMEOUTESCALATIONHANDLER_H

#include <iostream>
#include "ApprovalHandler.h"
#include <string>

class TimeoutEscalationHandler : public ApprovalHandler{
    public:
        std::string getName() override;
    protected:
        bool handleRequest(Request&) override;
};

#endif