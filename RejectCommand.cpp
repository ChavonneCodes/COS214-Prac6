#include "RejectCommand.h"
RejectCommand::RejectCommand(WorkItem* r){
    receiver =r;
}
bool RejectCommand::execute(){
    return receiver->reject();
}
void RejectCommand::undo(){
    std::cout<<"Command undone"<<std::endl;
}
std::string RejectCommand::getDescription(){
    return  "WorkItem ID: "+receiver->getId()+" has been Rejected";
}