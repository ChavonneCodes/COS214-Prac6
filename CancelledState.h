#ifndef CANCELLEDSTATE_H
#define CANCELLEDSTATE_H

#include "WorkItemState.h"
class CancelledState: public WorkItemState{

    public:
       
       WorkItemState* clone();
}; 
#endif