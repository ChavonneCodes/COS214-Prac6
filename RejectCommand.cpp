#include "RejectCommand.h"
RejectCommand::RejectCommand(WorkItem* r){
    receiver =r;
}
bool RejectCommand::execute(){
    if(!receiver){
        return false;
    }
    WorkItemMemento snapshot = receiver->createMemento();
    bool successful = false;
    try{
    successful = receiver->reject();
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
void RejectCommand::undo(){
     if(!receiver|| myHistory.empty()){
        return;
    }
    receiver->restoreMemento(&myHistory.back());
    myHistory.pop_back();
    std::cout<<"Rejection undone"<<std::endl;
}
std::string RejectCommand::getDescription(){
    return  "WorkItem ID: "+receiver->getId()+" has been Rejected";
}
std::string RejectCommand::getInstanceId(){
    return receiver->getInstanceId();
}
