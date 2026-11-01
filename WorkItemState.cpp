#include "WorkItemState.h"
#include "WorkItem.h"

//these are helper functions that allow concrete states to call private functions by allowing base class
//to act as a friend class. This helps encapsulate state changes as commands

void WorkItemState::transition(WorkItem* item, WorkItemState* next) { item->changeState(next); }
void WorkItemState::bind(WorkItem* item, Participant* p)            { item->bindParticipant(p); }
void WorkItemState::release(WorkItem* item){item->releaseParticipant();};
void WorkItemState::reduceParticipantCount(WorkItem* item){item->lowerWorkLoad();}
