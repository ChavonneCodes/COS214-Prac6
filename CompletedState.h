#ifndef COMPLETEDSTATE_H
#define COMPLETEDSTATE_H

#include "WorkItemState.h"
class CompletedState: public WorkItemState{

    public:
        
       WorkItemState* clone();
       std::string getName()override;
}; 
#endif