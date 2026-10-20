#ifndef ROLEAPPROVALHANDLER_H
#define ROLEAPPROVALHANDLER_H

#include "ApprovalHandler.h"
#include <iostream>
#include <string>

class RoleApprovalHandler : public ApprovalHandler {
    public:
        RoleApprovalHandler(const std::string&);
        std::string getName() override;
    protected:
        bool handleRequest(Request&) override;
    private:
        std::string roleId;
};

#endif