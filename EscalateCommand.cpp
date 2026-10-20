#include "EscalateCommand.h"

EscalateCommand::EscalateCommand(WorkItem* r, std::string& id){
    receiver =r;
    targetRoleId = id;
}
bool EscalateCommand::execute(){
    return receiver->escalated();
}
void EscalateCommand::undo(){
    std::cout<<"Command undone"<<std::endl;
}
std::string EscalateCommand::getDescription(){
    return  "WorkItem ID: "+ receiver->getId() +" has been Escalateed to pariticpant: "+ targetRoleId;}