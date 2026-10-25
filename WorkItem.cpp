#include "WorkItem.h"
#include "CreatedState.h"
#include "AvailableState.h"
#include "AssignedState.h"
#include "InProgressState.h"
#include "CompletedState.h"
#include "FailedState.h"
#include "RejectedState.h"
#include "CancelledState.h"
#include "EscalatedState.h"
#include <iostream>

WorkItem::WorkItem(const std::string& instanceId, ActivityDefinition* def)
    : instanceId(instanceId), definition(def), currentState(new CreatedState()) {}

// ---- State Pattern Support ----

std::string WorkItem::getState() const { 
    return currentState->getName(); 
}

void WorkItem::changeState(WorkItemState* newState) {
    if (newState == nullptr) return;
    
    std::string oldStateName = currentState->getName();
    delete currentState; 
    currentState = newState;
    std::string newStateName = currentState->getName();
    
    // Observer: Publish event when state changes!
    publish("STATE_CHANGED", oldStateName + " -> " + newStateName);
}

void WorkItem::setState(const std::string& newState) {
    // Compatibility layer: Maps strings to State objects for the Observer tests
    if (newState == "IDLE" || newState == "CREATED") changeState(new CreatedState());
    else if (newState == "AVAILABLE") changeState(new AvailableState());
    else if (newState == "ASSIGNED") changeState(new AssignedState());
    else if (newState == "RUNNING" || newState == "IN_PROGRESS") changeState(new InProgressState());
    else if (newState == "SUCCESS" || newState == "COMPLETED") changeState(new CompletedState());
    else if (newState == "FAILED") changeState(new FailedState());
    else if (newState == "REJECTED") changeState(new RejectedState());
    else if (newState == "CANCELLED") changeState(new CancelledState());
    else if (newState == "ESCALATED") changeState(new EscalatedState());
    else std::cout << "Unknown state string: " << newState << std::endl;
}

WorkItemState* WorkItem::getCurrentState() {
    return currentState;
}

// ---- Strategy & Participant Support ----

std::string WorkItem::getRequiredRoleId() const { return definition->getRequiredRoleId(); }

Participant* WorkItem::getAssignedParticipant() const { return assignedParticipant; }

void WorkItem::setParticipant(Participant* p) {
    if (p == nullptr) return;
    if (assignedParticipant) assignedParticipant->closeOpenItem(); // reassignment
    
    assignedParticipant = p;
    assignedParticipant->addOpenItem();
}

bool WorkItem::assignTo(Participant* participant) {
    if (!participant) return false;
    setParticipant(participant);
    changeState(new AssignedState());
    return true;
}

// ---- State Transition Delegation ----

bool WorkItem::assign(Participant* p) { return currentState->assign(this, p); }
bool WorkItem::start() { return currentState->start(this); }
bool WorkItem::complete() { return currentState->complete(this); }
bool WorkItem::reject() { return currentState->reject(this); }
bool WorkItem::cancel() { return currentState->cancel(this); }
bool WorkItem::escalated() { return currentState->escalated(this); }
bool WorkItem::fail() { return currentState->fail(this); }
bool WorkItem::makeAvailable() { return currentState->makeAvailable(this); }

// ---- Observer Support ----

std::string WorkItem::getId() const { return definition->getId(); }

void WorkItem::setEventBus(EventBus* bus) { eventBus = bus; }

std::string WorkItem::getInstanceId() const { return instanceId; }

void WorkItem::publish(const std::string& eventName, const std::string& payload) {
    if (!eventBus) return; 
    eventBus->notify(WorkFlowEvent(eventName, instanceId, getId(), payload));
}

void WorkItem::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "[WorkItem] " << definition->getName()  
              << " (State: " << currentState->getName() << ")";
    if (assignedParticipant) {
        std::cout << " (Assigned: " << assignedParticipant->getName() << ")";
    }
    std::cout << "\n";
}

WorkItem::~WorkItem() {
    delete currentState;
}