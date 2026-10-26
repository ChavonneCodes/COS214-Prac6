

#include "RuntimeComponent.h"
#include <iostream>

void RuntimeComponent::addChild(RuntimeComponent* /*child*/) {
    std::cout << "Cannot add child to a leaf runtime component." << std::endl;
}

std::vector<RuntimeComponent*> RuntimeComponent::getChildren() const {
    return {};
}