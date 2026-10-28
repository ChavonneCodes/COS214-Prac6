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


/**
 * @brief Creates a snapshot of this work item's current data and state.
 *
 * The WorkItemMemento constructor clones the state pointer, so the
 * snapshot remains valid after the work item changes state.
 *
 * @return A memento representing the current snapshot.
 */
WorkItemMemento WorkItem::createMemento(){
    return WorkItemMemento(
                            definition->getId(),
                            getId(),
                            data,
                            currentState
                        );
}

/**
 * @brief Restores the data and state from a saved memento.
 *
 * The snapshot is checked to ensure it belongs to this work item
 * and its associated activity. The state is cloned before replacing
 * the current state.
 *
 * @param memento Pointer to the memento to restore.
 *        If nullptr, no action is performed.
 */
void WorkItem::restoreMemento(WorkItemMemento* memento){
    if(memento == nullptr){
        return;
    }

    //make sure the snapshot belongs to this work item
    if(memento->workItemId != getId()){
        return;
    }

    if(memento->activityId != definition->getId()){
        return;
    }

    WorkItemState* restoredState = memento->state->clone();

    data = memento->dataSnapshot;

    //restore the saved state
    changeState(restoredState);
}


std::string WorkItem::getState() const { return currentState->getName(); }
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