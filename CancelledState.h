#ifndef CANCELLEDSTATE_H
#define CANCELLEDSTATE_H

#include "WorkItemState.h"
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