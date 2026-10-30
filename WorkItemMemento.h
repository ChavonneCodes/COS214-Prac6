#ifndef WORKITEMMEMENTO_H
#define WORKITEMMEMENTO_H

#include <iostream>
#include "WorkItemState.h"
#include "Data.h"
#include <string>

/* enum StateKind {
    IDLE,
    RUNNING,
    SUCCESS,
    FAILED,
    AVAILABLE,
    ASSIGNED,
    REJECTED,
    ESCALATED};
*/

class WorkItemMemento{
    public:
        /**
         * @brief Constructs an empty memento.
         *
         * @details The memento has no associated IDs or saved state.
         */
        WorkItemMemento();

        /**
         * @brief Constructs a memento containing a snapshot of a work item.
         *
         * @param activityId ID of the activity associated with the work item.
         * @param workItemId ID of the work item being saved.
         * @param snapshot Data to preserve in the memento.
         * @param state Current state to snapshot. The memento clones this state.
         */
        WorkItemMemento(std::string,std::string,Data&,WorkItemState*);

        /**
         * @brief Copy constructor.
         *
         * Creates an independent copy of the saved state.
         *
         * @param other Memento to copy.
         */
        WorkItemMemento(const WorkItemMemento& other);

        /**
         * @brief Destroys the memento and releases its owned state.
         */
        ~WorkItemMemento();

        /**
         * @brief Copy assignment operator.
         *
         * Replaces the current snapshot with an independent copy of another.
         *
         * @param other Memento to copy.
         * @return Reference to this memento.
         */
        WorkItemMemento& operator=(const WorkItemMemento& other);

    private:
        friend class WorkItem;
        friend class WorkItemMementoTest;

        //getters
        std::string getActivityId() const;
        std::string getWorkItemId() const;
        WorkItemState* getStateKind() const;
        Data getDataSnapshot() const;

        std::string activityId;
        std::string workItemId;
        WorkItemState* state;
        Data dataSnapshot;

};

#endif