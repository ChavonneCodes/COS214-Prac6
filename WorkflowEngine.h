#ifndef WORKFLOWENGINE_H
#define WORKFLOWENGINE_H
#include <map>
#include <string>
#include <vector>
#include "WorkflowAction.h"
#include "InMemoryDefinitionRepository.h"
#include "EventBus.h"
#include "HistoryManager.h"
#include "AuditObserver.h"
#include "AssignmentStrategy.h"
#include "Participant.h"
#include "WorkflowInstance.h"

/**
 * @brief Single entry point to TIVIDY (Facade pattern, role: Facade).
 *
 * Clients (the TUI and the scenarios) talk only to the engine. It hides the
 * subsystems behind it: the definition repository, the instance factory, the
 * event bus, the command history and the audit log.
 *
 * The engine does not own the subsystems it is given. It does own the
 * instances it starts and the commands submitted to it.
 */
class WorkflowEngine{
private:
    InMemoryDefinitionRepository* repository = nullptr; ///< Where definitions are loaded from (not owned).
    EventBus* eventBus = nullptr;                       ///< Bus that engine events are published on (not owned).
    HistoryManager* history = nullptr;                  ///< Executed commands, used for undo (not owned).
    AuditObserver* audit = nullptr;                     ///< Audit log observer (not owned).

    AssignmentStrategy* assignment = nullptr;           ///< Picks who gets each new work item (not owned, may be null).
    std::vector<Participant*> candidates;               ///< Participants the strategy chooses from (not owned).

    std::map<std::string, WorkflowInstance*> instances; ///< Started instances by id (owned).
    std::vector<WorkflowAction*> submitted;             ///< Every command submitted (owned).
    int instanceCount = 0;                              ///< Used to number new instance ids.

public:
    WorkflowEngine() = default;

    /**
     * @brief Creates an engine connected to its subsystems.
     * @param repository Definition repository (not owned).
     * @param eventBus   Event bus (not owned).
     * @param history    Command history (not owned).
     * @param audit      Audit observer (not owned).
     */
    WorkflowEngine(InMemoryDefinitionRepository* repository, EventBus* eventBus,
                   HistoryManager* history, AuditObserver* audit);

    /** @brief Deletes the instances started and the commands submitted. */
    ~WorkflowEngine();

    WorkflowEngine(const WorkflowEngine&) = delete;
    WorkflowEngine& operator=(const WorkflowEngine&) = delete;

    /**
     * @brief Sets who new work items are assigned to (Strategy pattern).
     * @param strategy   Assignment rule (not owned, may be null for no assignment).
     * @param candidates Participants the rule chooses from (not owned).
     */
    void setAssignment(AssignmentStrategy* strategy, const std::vector<Participant*>& candidates);

    /**
     * @brief Creates and starts a new instance of a workflow definition.
     * @param defId   Id of the definition to run.
     * @param version Version to run (the instance stays pinned to it).
     * @return Id of the new instance, or an empty string if the definition was not found.
     */
    std::string startWorkflow(const std::string& defId, int version);

    /**
     * @brief Looks up a running workflow instance.
     * @param instanceId Id returned by startWorkflow().
     * @return The instance, or nullptr if there is none with that id.
     */
    WorkflowInstance* getInstance(const std::string& instanceId);

    /**
     * @brief Executes a command (Command pattern, role: Invoker).
     *
     * The engine takes ownership of the command. Commands that succeed are
     * recorded in the history.
     * @param action Command to execute.
     * @return true if the command succeeded.
     */
    bool submit(WorkflowAction* action);

    /**
     * @brief Undoes the most recently executed command.
     */
    void undoLast();

    /**
     * @brief Returns the commands executed on an instance.
     * @param instanceId Id of the instance.
     * @return The executed commands, oldest first.
     */
    std::vector<WorkflowAction*> getHistory(const std::string& instanceId);

    /**
     * @brief Returns the audit log entries for an instance.
     * @param instanceId Id of the instance.
     * @return The audit entries, oldest first.
     */
    std::vector<std::string> getAuditTrail(const std::string& instanceId);

};
#endif
