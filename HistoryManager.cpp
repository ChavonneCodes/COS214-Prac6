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
vector<WorkflowAction*> HistoryManager::getAll(){
    return executed;
}

/**
 * @brief Saves a snapshot in the history.
 *
 * The memento is copied into the vector. Its copy constructor ensures
 * that the saved state is independent of the original memento.
 *
 * @param action Memento to save.
 */
void HistoryManager::push(WorkflowAction* action){
    executed.push_back(action);
}


/**
 * @brief Removes and returns the most recently saved snapshot.
 *
 * @return Most recent memento, or a default memento if history is empty.
 */
WorkflowAction* HistoryManager::pop(){
    WorkflowAction* a1;
    if(!executed.empty()){
        a1 = executed.at(executed.size() - 1);
        executed.pop_back();
        return a1;
    } 

    a1 = nullptr;
    return a1;
}

