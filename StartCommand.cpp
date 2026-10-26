#include "StartCommand.h"

StartCommand::StartCommand(WorkItem* r){
    receiver =r;
}
bool StartCommand::execute(){
    return receiver->start();
}
void StartCommand::undo(){
    std::cout<<"Command undone"<<std::endl;
}
std::string StartCommand::getDescription(){
    return  "WorkItem ID: "+receiver->getId() +" has started";
}
std::string StartCommand::getInstanceId(){
    return receiver->getInstanceId();
}
