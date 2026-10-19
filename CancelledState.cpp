#include "CancelledState.h"

WorkItemState* CancelledState::clone(){
    return new CancelledState(*this);
}