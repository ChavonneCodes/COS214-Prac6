#include "TimeoutEscalationHandler.h"
using namespace std;

string TimeoutEscalationHandler::getName(){
    return "Timeout Escalation Handler";
}

bool TimeoutEscalationHandler::handleRequest(Request& request){
    if(request.getAction() == "TIMEOUT"){
        //escalation logic
        return true;
    }

    return false;
}