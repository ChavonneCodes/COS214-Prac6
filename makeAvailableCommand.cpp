#include "makeAvailableCommand.h"
makeAvailableCommand::makeAvailableCommand(WorkItem* r){
    receiver =r;
}
bool makeAvailableCommand::execute(){
    if(!receiver){
        return false;
    }
    WorkItemMemento snapshot = receiver->createMemento();
    bool successful = false;
    try{
    successful = receiver->makeAvailable();
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
void makeAvailableCommand::undo(){
     if(!receiver|| myHistory.empty()){
        return;
    }
    receiver->restoreMemento(&myHistory.back());
    myHistory.pop_back();
    std::cout<<"Make Available undone"<<std::endl;
}
std::string makeAvailableCommand::getDescription(){
    return  "WorkItem ID: "+receiver->getId()+" has been made Available";
}
std::string makeAvailableCommand::getInstanceId(){
    return receiver->getInstanceId();
}
