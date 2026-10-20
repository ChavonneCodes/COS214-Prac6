#include "TimeoutEscalationHandler.h"
using namespace std;

string TimeoutEscalationHandler::getName(){
    return "TimeoutEscalationHandler";
}

bool TimeoutEscalationHandler::handleRequest(Request& request){
    if(request.getAction() == "TIMEOUT"){
        //escalation logic
        return true;
    }

    return false;
}