#ifndef COMPLETEDSTATE_H
#define COMPLETEDSTATE_H

#include "WorkItemState.h"
class CompletedState: public WorkItemState{

    public:
    /**
     
     * @return copy of CompletedState object, used to stroe state in memento
     */
       WorkItemState* clone();
       /**
     * @return name of CompletedState as a string
     */
       std::string getName()override;
}; 
#endif