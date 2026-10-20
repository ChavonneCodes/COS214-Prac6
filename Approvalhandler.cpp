
#include "ApprovalHandler.h"

using namespace std;

string ApprovalHandler::getName() {
    return "ApprovalHandler";
}

void ApprovalHandler::setNext(ApprovalHandler* handler) {
    next = handler;
}

bool ApprovalHandler::handleRequest(Request& ) {
    // The base handler cannot approve requests itself.
    return false;
}

bool ApprovalHandler::authorise(Request& request) {

    // Step 1: Try to handle the request here.
    if (handleRequest(request)) {
        cout << getName() << " handled the request.\n";
        return true;
    }

    // Step 2: Pass the request to the next handler.
    if (next != nullptr) {
        return next->authorise(request);
    }

    // Step 3: No handler could handle the request.
    cout << "No handler could authorise the request.\n";
    return false;
}
