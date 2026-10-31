#include "WorkItemState.h"
#include "WorkItem.h"

void WorkItemState::transition(WorkItem* item, WorkItemState* next) { item->changeState(next); }
void WorkItemState::bind(WorkItem* item, Participant* p)            { item->bindParticipant(p); }
void WorkItemState::release(WorkItem* item){item->releaseParticipant();};