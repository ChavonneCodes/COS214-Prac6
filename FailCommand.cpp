#include "FailCommand.h"
FailCommand::FailCommand(WorkItem* r){
    receiver =r;
}
bool FailCommand::execute(){
    if(!receiver){
        return false;
    }
    WorkItemMemento snapshot = receiver->createMemento();
    bool successful = false;
    try{
    successful = receiver->fail();
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
void FailCommand::undo(){
     if(!receiver|| myHistory.empty()){
        return;
    }
    receiver->restoreMemento(&myHistory.back());
    myHistory.pop_back();
    std::cout<<"Failure undone"<<std::endl;
}
std::string FailCommand::getDescription(){
    return  "WorkItem ID: "+receiver->getId()+" has been Failed";
}
std::string FailCommand::getInstanceId(){
    return receiver->getInstanceId();
}
