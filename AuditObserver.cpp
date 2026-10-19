/**
 * @file AuditObserver.cpp
 * @brief Implementation of AuditObserver (Observer pattern: ConcreteObserver).
 */
#include "AuditObserver.h"
#include <iostream>

void AuditObserver::update(const WorkFlowEvent& event) {
    log.push_back("#" + std::to_string(event.getSequence()) + " [" +
                  event.getEventName() + "] instance=" + event.getInstanceId() +
                  " item=" + event.getWorkItemId() + " : " + event.getPayload());
}

std::vector<std::string> AuditObserver::getLog() const {
    return log;
}

void AuditObserver::printLog() const {
    std::cout << "=== audit trail ===\n";
    for (const std::string& line : log) {
        std::cout << line << "\n";
    }
}
