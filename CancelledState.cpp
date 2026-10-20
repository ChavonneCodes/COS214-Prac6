#include "CancelledState.h"

WorkItemState* CancelledState::clone(){
    return new CancelledState(*this);
}
 std::string CancelledState::getName(){
   return "Cancelled";
 }