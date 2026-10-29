#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include "WorkflowAction.h"
#include <iostream>
#include <vector>

class HistoryManager{
    public:
        /**
         * @brief Saves a snapshot in the history.
         *
         * The memento is copied into the vector. Its copy constructor ensures
         * that the saved state is independent of the original memento.
         *
         * @param action Memento to save.
         */
        void push(WorkflowAction*);

        /**
         * @brief Removes and returns the most recently saved snapshot.
         *
         * @return Most recent memento, or a default memento if history is empty.
         */
        WorkflowAction* pop();

        /**
         * @brief Returns a copy of all stored mementos.
         *
         * @return Vector containing the saved snapshots.
         */
        std::vector<WorkflowAction*> getAll();

    private:
        std::vector<WorkflowAction*> executed;  
};

#endif