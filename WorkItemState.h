/**
 * @file WorkItemState.h
 * @brief implements WorkItemState
 */
#ifndef WORKITEMSTATE_H
#define WORKITEMSTATE_H
class WorkItem;
class Participant;
#include <iostream>
#include <string>
/**
 * @class WorkItemState
 * @brief This is the State participant for a work item's life cycle. Every action a
 * work item can be asked to perform (assign, start, complete, reject, cancel,
 * escalate, fail, makeAvailable) is a virtual method that refuses by default:
 * it prints "Invalid State Transition" and returns false. Each concrete state
 * overrides only the actions it allows, so the legal transitions are defined
 * by which methods a state overrides and not by conditionals in WorkItem. The
 * class also declares the protected helpers (transition, bind, release,
 * finish) through which a state changes the item's state, owner and workload,
 * and the clone() used to save snapshots for the Memento.
 */



class WorkItemState{
    
    protected:
    /**
         * @brief helper to access bindParticipant in context class
         * @param item workItem that must undergo a state change
         * @param next the next state it must transition to
         */
    void transition(WorkItem* item, WorkItemState* next);
    /**
         * @brief helper to access changeState in context class
         * @param item workItem that must undergo a state change
         * @param p participant that must be bound
         */
    void bind(WorkItem* item, Participant* p);
    /**
         * @brief helper to access releaseParticipant in context class
         * @param item workItem that must undergo a state change
         * 
         */
    void release(WorkItem* item);
    /**
         * @brief helper to access lowerWorkLoad in context class
         * @param item workItem that must undergo a state change
         * 
         */
    void reduceParticipantCount(WorkItem* item);
    public: 
        /**
         * @brief default implementation when a state cannot transition to AssignedState
         * @return false, outputs default error message
         */
        virtual  bool assign(WorkItem*, Participant* ){std::cout<<"Invalid State Transition"<<std::endl;return false;};
        /**
         * @brief default implementation when a state cannot transition to InProgressState
         * @return false, outputs default error message
         */
        virtual bool start(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false; };
        /**
         * @brief default implementation when a state cannot transition to assign CompletedState
         * @return false, outputs default error message
         */
        virtual bool complete(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false;};
        /**
         * @brief default implementation when a state cannot transition to assign RejectedState
         *
         * @return false, outputs default error message
         */

        virtual bool reject(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false ;};
        /**
         * @brief default implementation when a state cannot transition to CancelledState
         *
         * @return false, outputs default error message
         */
        virtual bool cancel(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false ;};
        /**
         * @brief default implementation when a state cannot transition to EscalatedState
         *
         * @return false, outputs default error message
         */
        virtual bool escalated(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false;};
        /**
         * @brief default implementation when a state cannot transition to FailedState
         *
         * @return false, outputs default error message
         */
        virtual bool fail(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false;};
        /**
         * @brief default implementation when a state cannot transition to AvailableState
         *
         * @return false, outputs default error message
         */
        virtual bool makeAvailable(WorkItem* ) {std::cout<<"Invalid State Transition"<<std::endl;return false;};
        /**
         * @brief string description of concrete state
         * @return name of state
         */
        virtual std::string getName()=0;
        /**
         * @brief string description of concrete state
         * @return WorkItemState object
         */
        virtual WorkItemState* clone() = 0;
        /**
         * @brief virtual destructor
         * 
         */
        virtual ~WorkItemState(){};
        
};
#endif
