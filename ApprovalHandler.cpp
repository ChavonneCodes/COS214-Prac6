
#include "ApprovalHandler.h"
#include <iostream>

using namespace std;


string ApprovalHandler::getName() const{
    return "Approval Handler";
}


void ApprovalHandler::setNext(ApprovalHandler* handler) {
    next = handler;
}


HandlerResult ApprovalHandler::handleRequest(Request&) {
    // The base handler cannot approve requests itself.
    return HandlerResult::Continue;
}


HandlerResult ApprovalHandler::authorise(Request& request) {

    HandlerResult result = handleRequest(request);
    // Step 1: Try to handle the request here.
    if (result == HandlerResult::Handled) {
        cout << getName() << " handled the request.\n";
        return result;
    }

    if(result == HandlerResult::Rejected){
        cout << getName() << " rejected the Request.\n";
        return result;
    }

    // Step 2: Pass the request to the next handler.
    if (next != nullptr) {
        return next->authorise(request);
    }

    // Step 3: No handler could handle the request.
    cout << "No handler could authorise the request.\n";
    return HandlerResult::Rejected;
}
