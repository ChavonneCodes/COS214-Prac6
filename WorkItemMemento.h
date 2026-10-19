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
        std::string getWorkItemId();
        std::string getActivityId();
        WorkItemState* getStateKind();
        Data getDataSnapshot();

    private:
        std::string activityId;
        std::string workItemId;
        WorkItemState* state;
        Data dataSnapshot;

};

#endif