/**
 * @file AuditObserver.h
 * @brief Observer that keeps a permanent audit trail of workflow events.
 *
 * Part of the **Observer** pattern (role: ConcreteObserver).
 */
#ifndef AUDIT_OBSERVER_H
#define AUDIT_OBSERVER_H

#include <string>
#include <vector>
#include "WorkFlowObserver.h"

/**
 * @class AuditObserver
 * @brief Records every event it receives, in order, as one text line each.
 */
class AuditObserver : public WorkFlowObserver {
private:
    std::vector<std::string> log; ///< One formatted line per event received.

public:
    /**
     * @brief Appends a formatted line describing the event to the log.
     * @param event The event that was published.
     */
    void update(const WorkFlowEvent& event) override;

    /**

     * @brief Returns a copy of the audit log, oldest entry first.

     * @return A copy of the audit log, oldest entry first.

     */
    std::vector<std::string> getLog() const;

    /** @brief Prints the audit log to standard output. */
    void printLog() const;
};

#endif // AUDIT_OBSERVER_H
