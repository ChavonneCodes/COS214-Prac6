#include "StartCommand.h"

StartCommand::StartCommand(WorkItem* r){
    receiver =r;
}
bool StartCommand::execute(){
    if(!receiver){
        return false;
    }
    WorkItemMemento snapshot = receiver->createMemento();
    bool successful = false;
    try{
    successful = receiver->start();
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
void StartCommand::undo(){
     if(!receiver|| myHistory.empty()){
        return;
    }
    receiver->restoreMemento(&myHistory.back());
    myHistory.pop_back();
    
    std::cout<<"Mark as In_Progeess undone"<<std::endl;
}
std::string StartCommand::getDescription(){
    return  "WorkItem ID: "+receiver->getId() +" has started";
}
std::string StartCommand::getInstanceId(){
    return receiver->getInstanceId();
}
