#include "AssignCommand.h"

AssignCommand:: AssignCommand(WorkItem* r, Participant* p){
    receiver =r;
    participant =p;
}
bool AssignCommand::execute(){
    return receiver->assign(participant);
}
void AssignCommand::undo(){
    std::cout<<"Command undone"<<std::endl;
}
std::string AssignCommand::getDescription(){
    return  "WorkItem ID: "+ receiver->getId() +" has been Assigned";}