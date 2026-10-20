#include "CancelCommand.h"

CancelCommand::CancelCommand(WorkItem* r){
    receiver =r;
}
bool CancelCommand::execute(){
    return receiver->cancel();
}
void CancelCommand::undo(){
    std::cout<<"Command undone"<<std::endl;
}
std::string CancelCommand::getDescription(){
    return  "WorkItem ID: "+receiver->getId()+" has been Cancelled";
}