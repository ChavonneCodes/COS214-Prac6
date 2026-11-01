#ifndef COMPLETEDSTATE_H
#define COMPLETEDSTATE_H

#include "WorkItemState.h"
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