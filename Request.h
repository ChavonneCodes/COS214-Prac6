#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include "Data.h"
#include <string>

class Request{
    public:
        //constructor
        Request(const Data& data);
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
        std::string getAction() const;
        std::string getRequiredRoleId() const;
        std::string getWorkItemId() const;
        std::string getInstanceId() const;
        Data getData() const;
        std::string getRequesterId() const;
        std::string getCurrentState() const;

        //setters
        void setAction(const std::string&);
        void setCurrentState(const std::string&);
        void setData(const Data&);
        void setReason(const std::string&);
        void setRequiredRoleId(const std::string&);
        
        //validator
        bool isValid();


    private:
        std::string action;
        std::string WorkItemId;
        std::string instanceId;
        std::string requiredRoleId;
        Data data;
        std::string requestId;
        std::string requesterId;
        std::string reason;
        std::string currentState; //or state object
};

#endif