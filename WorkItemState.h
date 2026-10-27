#ifndef WORKITEMSTATE_H
#define WORKITEMSTATE_H
class WorkItem;
class Participant;
#include <iostream>
#include <string>

/**
 * @brief State participant. Returns default values for invalid state transitions. states apply variable criteria 
 */
class WorkItemState{
    public: 
        /**
         * @brief default implementation when a state cannot transition to AssignedState
         * @param 
         * @return
         */
        virtual  bool assign(WorkItem*, Participant* ){std::cout<<"Invalid State Transition"<<std::endl;return false;};
        /**
         * @brief default implementation when a state cannot transition to InProgressState
         * @param WorkItem* work item which undergoes state changes
         * @param Participant* the participant in charge of handling a work item
         * @return false, outputs default error message
         */
        virtual bool start(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false; };
        /**
         * @brief default implementation when a state cannot transition to assign CompletedState
         * @param WorkItem* workitem which undergoes state changes
         * @return false, outputs default error message
         */
        virtual bool complete(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false;};
        /**
         * @brief default implementation when a state cannot transition to assign RejectedState
         * @param WorkItem* work item which undergoes state changes
         * @return false, outputs default error message
         */

        virtual bool reject(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false ;};
        /**
         * @brief default implementation when a state cannot transition to CancelledState
         * @param WorkItem* work item which undergoes state changes
         * @return false, outputs default error message
         */
        virtual bool cancel(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false ;};
        /**
         * @brief default implementation when a state cannot transition to EscalatedState
         * @param WorkItem* work item which undergoes state changes
         * @return false, outputs default error message
         */
        virtual bool escalated(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false;};
        /**
         * @brief default implementation when a state cannot transition to FailedState
         * @param WorkItem* work item which undergoes state changes
         * @return false, outputs default error message
         */
        virtual bool fail(WorkItem* ){std::cout<<"Invalid State Transition"<<std::endl;return false;};
        /**
         * @brief default implementation when a state cannot transition to AvailableState
         * @param WorkItem* work item which undergoes state changes
         * @return false, outputs default error message
         */
        virtual bool makeAvailable(WorkItem* ) {std::cout<<"Invalid State Transition"<<std::endl;return false;};
        /**
         * @brief string description of concrete state
         * @return name of state
         */
        virtual std::string getName()=0;
        virtual WorkItemState* clone() = 0;
        virtual ~WorkItemState(){};
        
};
#endif
