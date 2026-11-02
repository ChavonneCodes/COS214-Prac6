/**
 * @file CompletedState.h
 * @brief Defines the CompletedState class.
 */
#ifndef COMPLETEDSTATE_H
#define COMPLETEDSTATE_H

#include "WorkItemState.h"
/**
 * @class CompletedState
 * @brief ConcreteState for a work item that has finished successfully. It is a final
 * state: it overrides no actions, so every action is refused. The owner stays
 * on record, but the item no longer counts towards their workload.
 */
class CompletedState: public WorkItemState{

    public:
    /**
     * @return copy of the CompletedState object
     */
       WorkItemState* clone()override;
       /**
     * @return name of CompletedState as a string
     */
       std::string getName()override;
}; 
#endif