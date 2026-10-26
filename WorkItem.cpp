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
#include "RuntimeVisitor.h"

WorkItem::WorkItem(const std::string& instanceId, ActivityDefinition* def)
    : instanceId(instanceId), definition(def), assignedParticipant(nullptr), 
      currentState(new CreatedState()), eventBus(nullptr) {} // No more string state

bool WorkItem::assignTo(Participant* participant) {
    if (!participant) return false;
    if (assignedParticipant) assignedParticipant->closeOpenItem(); // reassignment
    
    assignedParticipant = participant;
    assignedParticipant->addOpenItem();

    // Enforce proper state transitions: Created -> Available -> Assigned
    if (currentState->getName() == "Created") {
        currentState->makeAvailable(this); // Transitions to AvailableState
    }
    return currentState->assign(this, participant); // Transitions to AssignedState
}

// ---- State Pattern Support ----

std::string WorkItem::getState() const { 
    return currentState->getName(); 
}

// This method is now just a compatibility wrapper for tests that call setState("...")
void WorkItem::setState(const std::string& newState) {
    if (newState == "Created" || newState == "CREATED" || newState == "IDLE") changeState(new CreatedState());
    else if (newState == "Available" || newState == "AVAILABLE") changeState(new AvailableState());
    else if (newState == "Assigned" || newState == "ASSIGNED") changeState(new AssignedState());
    else if (newState == "In_Progress" || newState == "RUNNING") changeState(new InProgressState());
    else if (newState == "Completed" || newState == "SUCCESS") changeState(new CompletedState());
    else if (newState == "Failed" || newState == "FAILED") changeState(new FailedState());
    else if (newState == "Rejected" || newState == "REJECTED") changeState(new RejectedState());
    else if (newState == "Cancelled" || newState == "CANCELLED") changeState(new CancelledState());
    else if (newState == "Escalated" || newState == "ESCALATED") changeState(new EscalatedState());
}

void WorkItem::changeState(WorkItemState* newState) {
    if (newState == nullptr) return;
    std::string oldStateName = currentState->getName();
    delete currentState; 
    currentState = newState;
    std::string newStateName = currentState->getName();
    
    // Publish the event here so we don't need the string member
    publish("STATE_CHANGED", oldStateName + " -> " + newStateName);
}

WorkItemState* WorkItem::getCurrentState() { 
    return currentState; 
}

// ---- Delegation to State Objects ----

bool WorkItem::assign(Participant* p) { return currentState->assign(this, p); }
bool WorkItem::start() { return currentState->start(this); }
bool WorkItem::complete() { return currentState->complete(this); }
bool WorkItem::reject() { return currentState->reject(this); }
bool WorkItem::cancel() { return currentState->cancel(this); }
bool WorkItem::escalated() { return currentState->escalated(this); }
bool WorkItem::fail() { return currentState->fail(this); }
bool WorkItem::makeAvailable() { return currentState->makeAvailable(this); }

// ---- Standard Methods ----

std::string WorkItem::getRequiredRoleId() const { return definition->getRequiredRoleId(); }
Participant* WorkItem::getAssignedParticipant() const { return assignedParticipant; }
std::string WorkItem::getId() const { return definition->getId(); }

void WorkItem::setEventBus(EventBus* bus) { eventBus = bus; }
std::string WorkItem::getInstanceId() const { return instanceId; }

void WorkItem::publish(const std::string& eventName, const std::string& payload) {
    if (!eventBus) return;  
    eventBus->notify(WorkFlowEvent(eventName, instanceId, getId(), payload));
}

void WorkItem::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    // Uses currentState->getName() directly instead of a string member
    std::cout << indent << "[WorkItem] " << definition->getName()  
              << " (State: " << currentState->getName() << ")";
    if (assignedParticipant) {
        std::cout << " (Assigned: " << assignedParticipant->getName() << ")";
    }
    std::cout << "\n";
}

void WorkItem::setParticipant(Participant* p){
    if (p == nullptr) return;
    assignedParticipant = p;
}

std::string WorkItem::getName() const { return definition->getName(); }

void WorkItem::accept(RuntimeVisitor& visitor) { visitor.visitWorkItem(*this); }

WorkItem::~WorkItem(){
    delete currentState;
}