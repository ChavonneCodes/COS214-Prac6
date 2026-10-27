/**
 * @file WorkItemMemento.cpp
 * @brief Implements the Memento pattern for WorkItem snapshots.
 *
 * A memento stores a work item's identifiers, data snapshot, and state.
 * The saved state is cloned so that it remains valid independently
 * of subsequent changes to the original WorkItem.
 */

#include "WorkItemMemento.h"
using namespace std;

/**
 * @brief Constructs an empty memento.
 *
 * @details The memento has no associated IDs or saved state.
 */
WorkItemMemento::WorkItemMemento() : activityId("") , workItemId("") , state(nullptr) , dataSnapshot(""){
}

/**
 * @brief Constructs a memento containing a snapshot of a work item.
 *
 * @param activityId ID of the activity associated with the work item.
 * @param workItemId ID of the work item being saved.
 * @param snapshot Data to preserve in the memento.
 * @param state Current state to snapshot. The memento clones this state.
 */

WorkItemMemento::WorkItemMemento(string activityId,string workItemId,Data& snapshot,WorkItemState* state) : activityId(activityId) , workItemId(workItemId) , state(state != nullptr ? state->clone() : nullptr) , dataSnapshot(snapshot){
}

/**
 * @brief Copy constructor.
 *
 * Creates an independent copy of the saved state.
 *
 * @param other Memento to copy.
 */
WorkItemMemento::WorkItemMemento(const WorkItemMemento& other)
    : activityId(other.activityId),
      workItemId(other.workItemId),
      state(other.state != nullptr ? other.state->clone() : nullptr),
      dataSnapshot(other.dataSnapshot)
{
}

/**
 * @brief Copy assignment operator.
 *
 * Replaces the current snapshot with an independent copy of another.
 *
 * @param other Memento to copy.
 * @return Reference to this memento.
 */
WorkItemMemento& WorkItemMemento::operator=(const WorkItemMemento& other){
    if(this != &other){
        WorkItemState* copiedState = other.state != nullptr ? other.state->clone() : nullptr;

        delete state;

        activityId = other.activityId;
        workItemId = other.workItemId;
        dataSnapshot = other.dataSnapshot;
        state = copiedState;
    }
    
    return *this;
}

/**
 * @brief Destroys the memento and releases its owned state.
 */
WorkItemMemento::~WorkItemMemento(){
    delete state;
}

/*
string WorkItemMemento::getWorkItemId(){
    return workItemId;
}

string WorkItemMemento::getActivityId(){
    return activityId;
}

WorkItemState* WorkItemMemento::getStateKind(){
    return state;
}

Data WorkItemMemento::getDataSnapshot(){
    return dataSnapshot;
}
*/