#include "CompletedState.h"

WorkItemState* CompletedState::clone(){
    return new CompletedState(*this);
}