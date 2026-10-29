#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include "Data.h"
#include <string>

/**
    *@brief Represents a request for a workflow approval action.
    *The Request class stores the information required to process
    *and validate an approval request within the workflow system.
    *This includes the requester's details, the target work item,
    *the required role, the current workflow state, and associated data.
*/

class Request{
    public:
        //constructor

        /** 
        *@brief Constructs a Request with the provided data.
        *Initializes the request with the supplied Data object.
        *Other request attributes are initialized to empty strings.
        *@param data The data associated with the request.
        */
        Request(const Data& data);

        /**
        *@brief Constructs a fully specified Request object.
        *Initializes the request with its identifying information, requested action, authorization requirements, and associated data.
        *@param requestId The unique identifier of the request.
        *@param action The action being requested.
        *@param workItemId The identifier of the target work item.
        *@param instanceId The identifier of the workflow instance.
        *@param requiredRoleId The identifier of the role required to
        *authorize the request.
        *@param requesterId The identifier of the person making the request.
        *@param reason The reason for submitting the request.
        *@param currentState The current state of the workflow.
        *@param data The data associated with the request.
        */
        Request(const std::string& requestId,
            const std::string& action,
            const std::string& workItemId,
            const std::string& instanceId,
            const std::string& requiredRoleId,
            const std::string& requesterId,
            const std::string& reason,
            const std::string& currentState,
            const Data& data);

        //getters
        /**
        *@brief Retrieves the requested action.
        *@return The action associated with the request.
        */
        std::string getAction() const;
        
        /**
        *@brief Retrieves the role required to authorize the request.
        *@return The required role identifier.
        */
        std::string getRequiredRoleId() const;
        
        /**
        *@brief Retrieves the identifier of the target work item.
        *@return The work item identifier.
        */
        std::string getWorkItemId() const;
        
        /**
        *@brief Retrieves the workflow instance identifier.
        *@return The workflow instance identifier.
        */
        std::string getInstanceId() const;
        /**
        *@brief Retrieves the data associated with the request.
        *@return A copy of the request's Data object.
        */
        Data getData() const;
        
        /**
        *@brief Retrieves the identifier of the requester.
        *@return The requester identifier.
        */
        std::string getRequesterId() const;

        /**
        *@brief Retrieves the current workflow state.
        *@return The current state of the workflow.
        */
        std::string getCurrentState() const;

        //setters
        /**
        *@brief Updates the requested action.
        *@param action The new action to assign to the request.
        */
        void setAction(const std::string&);

        /**
        *@brief Updates the current workflow state.
        *@param currentState The new workflow state.
        */
        void setCurrentState(const std::string&);

        /**
        *@brief Updates the data associated with the request.
        *@param data The new data to associate with the request.
        */
        void setData(const Data&);

        /**
        *@brief Updates the reason for the request.
        *@param reason The new reason for submitting the request.
        */
        void setReason(const std::string&);
        
        /**
        *@brief Updates the role required to authorize the request.
        *@param requiredRoleId The new required role identifier.
        */
        void setRequiredRoleId(const std::string&);
        
        /**
        *@brief Validates the request's required fields.
        *Checks that the request identifier, action, work item identifier,
        *workflow instance identifier, required role identifier, and requester identifier are not empty.
        *@return true if all required fields are non-empty; false otherwise.
        */
        bool isValid();

    private:
        std::string action;
        std::string workItemId;
        std::string instanceId;
        std::string requiredRoleId;
        Data data;
        std::string requestId;
        std::string requesterId;
        std::string reason;
        std::string currentState; //or state object
};

#endif