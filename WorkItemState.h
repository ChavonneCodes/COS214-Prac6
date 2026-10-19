#ifndef WORKITEMSTATE_H
#define WORKITEMSTATE_H
class WorkItem;
class Participant;
#include <iostream>

class WorkItemState{
    public: 
        virtual  void assign(WorkItem* item, Participant* p){std::cout<<"Invalid State Transition"<<std::endl;};
        virtual bool start(WorkItem* item){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool complete(WorkItem* item){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool reject(WorkItem* item){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool cancel(WorkItem* item){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool escalated(WorkItem* item){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool fail(WorkItem* item){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool makeAvailable(WorkItem* item) {std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual ~WorkItemState(){};
};
#endif
