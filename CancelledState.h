#ifndef CANCELLEDSTATE_H
#define CANCELLEDSTATE_H

#include "WorkItemState.h"
class CancelledState: public WorkItemState{

    public:
       /**

     * @return copy of CancelledState object, used to store state in memento (WorkItemMemento)
     */
       WorkItemState* clone();
       /**
     
     * @return name of CancelledState as a string
     */
       std::string getName()override;
}; 
#endif