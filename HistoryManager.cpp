/**
 * @file HistoryManager.cpp
 * @brief Implements storage and retrieval of work item snapshots.
 *
 * HistoryManager maintains a collection of mementos that can be used
 * to restore previous work item states.
 */
#include "HistoryManager.h"
using namespace std;

/**
 * @brief Returns a copy of all stored mementos.
 *
 * @return Vector containing the saved snapshots.
 */
vector<WorkItemMemento> HistoryManager::getAll(){
    return mementos;
}

/**
 * @brief Saves a snapshot in the history.
 *
 * The memento is copied into the vector. Its copy constructor ensures
 * that the saved state is independent of the original memento.
 *
 * @param action Memento to save.
 */
void HistoryManager::save(WorkItemMemento& action){
    mementos.push_back(action);
}


/**
 * @brief Removes and returns the most recently saved snapshot.
 *
 * @return Most recent memento, or a default memento if history is empty.
 */
WorkItemMemento HistoryManager::pop(){
    if(mementos.empty()){
        return WorkItemMemento();
    }

    WorkItemMemento last = mementos.back();
    mementos.pop_back();

    return last;
}

/**
 * @brief Returns the most recent snapshot without removing it.
 *
 * @return Most recent memento, or a default memento if history is empty.
 */
WorkItemMemento HistoryManager::getLast(){
    if(mementos.empty()){
        return WorkItemMemento();
    }

    return mementos.back();
}