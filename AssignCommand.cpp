#include "AssignCommand.h"

AssignCommand:: AssignCommand(WorkItem* r, Participant* p){
    receiver =r;
    participant =p;
}
bool AssignCommand::execute(){
    if(!receiver||!participant){
        return false;
    }
    WorkItemMemento snapshot = receiver->createMemento();
    bool successful = false;
    try{
    successful = receiver->assign(participant);
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
void AssignCommand::undo(){
    if(!receiver|| myHistory.empty()){
        return;
    }
    receiver->restoreMemento(&myHistory.back());
    myHistory.pop_back();
    std::cout<<"Assignment undone"<<std::endl;
}
std::string AssignCommand::getDescription(){
    return  "WorkItem ID: "+ receiver->getId() +" has been Assigned";}
std::string AssignCommand::getInstanceId(){
    return receiver->getInstanceId();
}
