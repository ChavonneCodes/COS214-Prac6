#ifndef TIMEOUTESCALATIONHANDLER_H
#define TIMEOUTESCALATIONHANDLER_H

#include "ApprovalHandler.h"
#include "WorkItem.h"

class TimeoutEscalationHandler : public ApprovalHandler {
public:
    std::string getName() const override;
    TimeoutEscalationHandler(WorkItem*);
    TimeoutEscalationHandler();

protected:
    HandlerResult handleRequest(Request& request) override;

private:
    WorkItem* workItem;
};

#endif