#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include "Data.h"
#include <string>

class Request{
    public:
        Request(std::string,std::string,std::string,std::string,Data&);
        std::string getAction();
        std::string getRequiredRoleId();

    private:
        std::string action;
        std::string WorkItemId;
        std::string instanceId;
        std::string requiredRoleId;
        Data data;
};

#endif