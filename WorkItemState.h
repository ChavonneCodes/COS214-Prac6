#ifndef WORKITEMSTATE_H
#define WORKITEMSTATE_H
class WorkItem;
class Participant;
#include <iostream>
#include <string>

class WorkItemState{
    public: 
        virtual  bool assign(WorkItem*, Participant* ){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool start(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool complete(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool reject(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool cancel(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool escalated(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool fail(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual bool makeAvailable(WorkItem* ) {std::cout<<"Invalid State Transition"<<std::endl; return false;};
        virtual std::string getName()=0;
                virtual ~WorkItemState(){};
        
};
#endif
