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
        WorkItemMemento();
        WorkItemMemento(std::string,std::string,Data&,WorkItemState*);
        WorkItemMemento(const WorkItemMemento& other);
        ~WorkItemMemento();
        WorkItemMemento& operator=(const WorkItemMemento& other);

    private:
        friend class WorkItem;

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