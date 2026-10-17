#ifndef WORKFLOW_COMPONENT_H
#define WORKFLOW_COMPONENT_H

#include <string>
#include <vector>
#include <iostream>
class WorkflowComponent {
    public:
        virtual ~WorkflowComponent() = default;
        virtual std::string getId() const = 0;
        virtual std::string getName() const = 0;
        virtual void print(int indentLevel=0)const=0;

        virtual void addChild(WorkflowComponent *child){
            std::cout<<"Cannot add child to a leaf component." << std::endl;
        }

        virtual std::vector<WorkflowComponent*> getChildren() const {
            return {};
        }

    };
    #endif