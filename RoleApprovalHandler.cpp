#include "RoleApprovalHandler.h"
#include <iostream>

using namespace std;

/**
 * @brief Constructs a RoleApprovalHandler for a specific role.
 *
 * Initialises the handler with the role ID it is responsible
 * for matching against approval requests.
 *
 * @param roleId The ID of the role this handler is responsible for.
 */
RoleApprovalHandler::RoleApprovalHandler(const string& roleId)
    : roleId(roleId) {
}

/**
 * @brief Returns the name of this handler.
 *
 * Provides the name used to identify this handler in the
 * approval chain.
 *
 * @return The name "RoleApprovalHandler".
 */
string RoleApprovalHandler::getName() const {
    return "RoleApprovalHandler";
}

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
HandlerResult RoleApprovalHandler::handleRequest(
    Request& request) {

    if (request.getAction() != "APPROVE") {
        return HandlerResult::Continue;
    }

    if (request.getRequiredRoleId() == roleId) {
        cout << "Required role matched: " << roleId << '\n';
        return HandlerResult::Handled;
    }

    cout << "This handler cannot process role: "
         << request.getRequiredRoleId() << '\n';

    return HandlerResult::Continue;
}