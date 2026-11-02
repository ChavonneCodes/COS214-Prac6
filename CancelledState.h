/**
 * @file CancelledState.h
 * @brief Defines the CancelledState class.
 */
#ifndef CANCELLEDSTATE_H
#define CANCELLEDSTATE_H

#include "WorkItemState.h"
/**
 * @class CancelledState
 * @brief ConcreteState for a work item that was abandoned before it finished. It is a
 * final state: it overrides no actions, so every action is refused. Any
 * previous owner stays on record, but the item no longer counts towards their
 * workload.
 */
class CancelledState: public WorkItemState{

    public:
       /**
     * @return copy of the CancelledState object
     */
       WorkItemState* clone()override;
       /**
     
     * @return name of CancelledState as a string
     */
       std::string getName()override;
}; 
#endif