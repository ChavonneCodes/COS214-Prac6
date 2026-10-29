#include "EscalateCommand.h"

EscalateCommand::EscalateCommand(WorkItem* r){
    receiver =r;
    
}
bool EscalateCommand::execute(){
    if(!receiver){
        return false;
    }
    WorkItemMemento snapshot = receiver->createMemento();
    bool successful = false;
    try{
    successful = receiver->escalated();
    }catch(...){
        receiver->restoreMemento(&snapshot);
        throw;
    }
    if(!successful){
        receiver->restoreMemento(&snapshot);
        return false;
    }
    myHistory.push_back(std::move(snapshot));

    return true;
    
}
void EscalateCommand::undo(){
    if(!receiver|| myHistory.empty()){
        return;
    }
    receiver->restoreMemento(&myHistory.back());
    myHistory.pop_back();
    std::cout<<"Escalation undone"<<std::endl;
}
std::string EscalateCommand::getDescription(){
    return  "WorkItem ID: "+ receiver->getId() +" has been Escalated. Please reassign task: ";}
std::string EscalateCommand::getInstanceId(){
    return receiver->getInstanceId();
}
