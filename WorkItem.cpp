
#include "WorkItem.h"
#include "CreatedState.h"

/**
 * @brief Constructs a work item in its initial state.
 *
 * @param instanceId ID of the workflow instance.
 * @param def Activity definition associated with this work item.
 */
WorkItem::WorkItem(const std::string& instanceId, ActivityDefinition* def)
    : instanceId(instanceId), definition(def), data(""),state("IDLE"),currentState(new CreatedState()) {}


// ---- Strategy support ----

std::string WorkItem::getRequiredRoleId() const { return definition->getRequiredRoleId(); }

Participant* WorkItem::getAssignedParticipant() const { return assignedParticipant; }

bool WorkItem::assignTo(Participant* participant) {
    if (!participant) return false;
    if (assignedParticipant) assignedParticipant->closeOpenItem();  // reassignment

    assignedParticipant = participant;
    assignedParticipant->addOpenItem();
    setState("ASSIGNED");
    return true;
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

std::string WorkItem::getId() const { return definition->getId(); }
std::string WorkItem::getState() const { return state; }
void WorkItem::setState(const std::string& newState) {
    std::string oldState = state;
    state = newState;
    publish("STATE_CHANGED", oldState + " -> " + newState);
}

// ---- Observer support ----

void WorkItem::setEventBus(EventBus* bus) { eventBus = bus; }

std::string WorkItem::getInstanceId() const { return instanceId; }

void WorkItem::publish(const std::string& eventName, const std::string& payload) {
    if (!eventBus) return;  // no bus attached: behave exactly as before
    eventBus->notify(WorkFlowEvent(eventName, instanceId, getId(), payload));
}

void WorkItem::print(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    std::cout << indent << "[WorkItem] " << definition->getName()  
              << " (State: " << state << ")";
    if (assignedParticipant) {
        std::cout << " (Assigned: " << assignedParticipant->getName() << ")";
    }
    std::cout << "\n";
}

void WorkItem::setParticipant(Participant* p){
    if (p==nullptr) return;
    assignedParticipant =p;
}
WorkItemState* WorkItem:: getCurrentState(){
    return currentState;
}
void WorkItem:: changeState(WorkItemState* newState){
    if(newState==nullptr){return;}
    delete currentState;
    currentState = newState;
}
bool WorkItem::assign(Participant* p){
        return currentState->assign(this, p);
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

/**
 * @brief Destroys the work item and releases its current state.
 */
    WorkItem::~WorkItem(){
        delete currentState;
        
    }

   

