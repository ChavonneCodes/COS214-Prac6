#include "CompletedState.h"

WorkItemState* CompletedState::clone(){
    return new CompletedState(*this);
}
 std::string CompletedState::getName(){
   return "Completed";
 }