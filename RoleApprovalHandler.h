#ifndef ROLEAPPROVALHANDLER_H
#define ROLEAPPROVALHANDLER_H

#include "ApprovalHandler.h"
#include <iostream>
#include <string>

class RoleApprovalHandler : public ApprovalHandler {
    public:
        /**
         * @brief Constructs a RoleApprovalHandler for a specific role.
         *
         * Initialises the handler with the role ID it is responsible
         * for matching against approval requests.
         *
         * @param roleId The ID of the role this handler is responsible for.
         */
        RoleApprovalHandler(const std::string&);

        /**
         * @brief Returns the name of this handler.
         *
         * Provides the name used to identify this handler in the
         * approval chain.
         *
         * @return The name "RoleApprovalHandler".
         */
        std::string getName() const override;

    protected:
        /**
         * @brief Processes an approval request by checking its required role.
         *
         * If the request action is not "APPROVE", the handler returns
         * HandlerResult::Continue so another handler can process it.
         *
         * If the request's required role ID matches this handler's role ID,
         * the handler reports the match and returns HandlerResult::Handled.
         *
         * If the role IDs do not match, the handler reports that it cannot
         * process the requested role and returns HandlerResult::Continue,
         * allowing the request to pass to the next handler.
         *
         * @param request Reference to the request being processed.
         * @return HandlerResult::Continue if the action is not APPROVE
         *         or the role does not match; HandlerResult::Handled if
         *         the required role matches.
         */
        HandlerResult handleRequest(Request&) override;
    private:
        std::string roleId;
};

#endif