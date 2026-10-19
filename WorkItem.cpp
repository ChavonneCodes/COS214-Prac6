
#include "WorkItem.h"

WorkItem::WorkItem(const std::string& instanceId, ActivityDefinition* def)
    : instanceId(instanceId), definition(def), state("IDLE"),currentState(new CreatedState()) {}

std::string WorkItem::getId() const { return definition->getId(); }
std::string WorkItem::getState() const { return state; }
void WorkItem::setState(const std::string& newState) { state = newState; }

void WorkItem::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "[WorkItem] " << definition->getName() 
              << " (State: " << state << ")\n";
}
void WorkItem:: changeState(WorkItemState* newState){
    if(newState==nullptr){return;}
    delete currentState;
    currentState = newState;
}
    bool WorkItem::assign(Participant* p){
        return currentState->assign(this,p);
    }
    bool WorkItem::start(){
        return currentState->start(this);
    }
    bool WorkItem::complete(){
        return currentState->complete(this);
    }
    bool WorkItem::reject(){
        return currentState->reject(this);
    }
    bool WorkItem::cancel(){
        return currentState->cancel(this);
    }
    bool WorkItem::escalated(){
        return currentState->escalated(this);
    }
    bool WorkItem::fail(){
        return currentState->fail(this);
    }
    bool WorkItem::makeAvailable(){
        return currentState->makeAvailable(this);
    }
    WorkItem::~WorkItem(){
        delete currentState;
        
    }