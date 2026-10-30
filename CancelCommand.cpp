#include "CancelCommand.h"

CancelCommand::CancelCommand(WorkItem* r){
    receiver =r;
}
bool CancelCommand::execute(){
    if(!receiver){
        return false;
    }
    WorkItemMemento snapshot = receiver->createMemento();
    bool successful = false;
    try{
    successful = receiver->cancel();
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
void CancelCommand::undo(){
    if(!receiver|| myHistory.empty()){
        return;
    }
    receiver->restoreMemento(&myHistory.back());
    myHistory.pop_back();
    std::cout<<"Cancellation undone"<<std::endl;
}
std::string CancelCommand::getDescription(){
    return  "WorkItem ID: "+receiver->getId()+" has been Cancelled";
}
std::string CancelCommand::getInstanceId(){
    return receiver->getInstanceId();
}
