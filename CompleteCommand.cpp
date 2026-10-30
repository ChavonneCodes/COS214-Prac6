#include "CompleteCommand.h"

CompleteCommand::CompleteCommand(WorkItem* r){
    receiver = r;
}
bool CompleteCommand::execute(){
    if(!receiver){
        return false;
    }
    WorkItemMemento snapshot = receiver->createMemento();
    bool successful = false;
    try{
    successful = receiver->complete();
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
void CompleteCommand::undo(){
    if(!receiver|| myHistory.empty()){
        return;
    }
    receiver->restoreMemento(&myHistory.back());
    myHistory.pop_back();
    std::cout<<"Completion undone"<<std::endl;
}
std::string CompleteCommand::getDescription(){
    return  "WorkItem ID: "+receiver->getId() +" has been Completed";}
std::string CompleteCommand::getInstanceId(){
    return receiver->getInstanceId();
}
