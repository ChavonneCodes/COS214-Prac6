#ifndef RUNTIME_COMPONENT_H
#define RUNTIME_COMPONENT_H

#include "Iterator.h"
#include <string>
#include <vector>
#include <iostream>

class RuntimeComponent : public Iterable<RuntimeComponent> {
public:
    virtual ~RuntimeComponent() = default;

    virtual std::string getId() const = 0;
    virtual std::string getState() const = 0;
    virtual void setState(const std::string& newState) = 0;
    virtual void print(int indentLevel = 0) const = 0;

    // Composite operations
    virtual void addChild(RuntimeComponent* ) {
        std::cout << "Cannot add child to a leaf runtime component." << std::endl;
    }
    virtual std::vector<RuntimeComponent*> getChildren() const {
        return {};
    }

    Iterator<RuntimeComponent>* createIterator(TraversalStrategy<RuntimeComponent>* strat) const override
    {
	    return new RuntimeComponentIterator(*this, strat);
    }
};

#endif //RUNTIME_COMPONENT_H
