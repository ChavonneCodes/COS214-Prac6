#include "CompleteCommand.h"

CompleteCommand::CompleteCommand(WorkItem* r){
    receiver = r;
}
bool CompleteCommand::execute(){
    return receiver->complete();
}
void CompleteCommand::undo(){
    std::cout<<"Command undone"<<std::endl;
}
std::string CompleteCommand::getDescription(){
    return  "WorkItem ID: "+receiver->getId() +" has been Completeed";}