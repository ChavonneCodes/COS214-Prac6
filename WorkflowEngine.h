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
#include "RuntimeVisitor.h"
#include "RoutingStrategy.h"

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
    /**
     * @brief Creates an engine with no subsystems attached.
     *
     * startWorkflow() returns an empty string until a repository is given, and
     * history, undo and audit calls do nothing. Mainly useful for tests.
     */
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

    /**
     * @brief Copying is disabled.
     *
     * The engine owns its instances and commands, so a copy would delete them twice.
     */
    WorkflowEngine(const WorkflowEngine&) = delete;

    /**
     * @brief Copy assignment is disabled, for the same reason as the copy constructor.
     * @return Not callable.
     */
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

    /**
     * @brief Runs a visitor over an instance's runtime tree (Visitor pattern).
     *
     * Lets clients add operations such as reports without reaching into the
     * tree themselves.
     * @param instanceId Id of the instance.
     * @param visitor    Operation to run on every node.
     * @return false (and nothing visited) if there is no instance with that id.
     */
    bool applyVisitor(const std::string& instanceId, RuntimeVisitor& visitor);

    /**
     * @brief Advances the workflow by one step.
     * 
     * Asks the router what comes next. If the next node is an activity,
     * returns its runtime WorkItem for the client to process. If the next
     * node is a sub-process, dispatches it automatically, runs the child
     * to completion, then recurses to the sub-process's successor.
     * 
     * @param instanceId   The running workflow instance.
     * @param completedId  The ID of the node that just finished ("" for the first step).
     * @param outcome      The outcome of the completed node.
     * @param router       The routing strategy to consult.
     * @return The next WorkItem for the client to process, or nullptr if done.
     */
    WorkItem* step(const std::string& instanceId,
                   const std::string& completedId,
                   const std::string& outcome,
                   RoutingStrategy& router);
    
    /**
     * @brief Automatically dispatches IDLE sub-processes and detects
     *        when running sub-processes have completed.
     * 
     * This method acts as the engine's structural dispatcher. It recursively
     * traverses the runtime tree of the specified workflow instance looking
     * for SubProcessInstance nodes.
     * 
     * It handles two scenarios automatically:
     * 
     * 1. **Dispatch (IDLE → RUNNING):**
     *    If a SubProcessInstance is in the "IDLE" state and has no child
     *    instance yet, the engine:
     *    - Loads the child definition from the repository (by ID + version)
     *    - Instantiates the child runtime tree via InstanceFactory
     *    - Creates and stores a nested WorkflowInstance
     *    - Attaches the event bus to all child work items
     *    - Transitions the sub-process node to "RUNNING"
     * 
     * 2. **Completion (RUNNING → COMPLETED):**
     *    If a SubProcessInstance is in the "RUNNING" state and its child
     *    instance has reached "COMPLETED", the engine marks the sub-process
     *    node as "COMPLETED", allowing the parent workflow to resume.
     * 
     * @note **Structural, not behavioural.** This method handles the
     *       *bureaucracy* of sub-processes (loading, instantiating, state
     *       transitions). It does **not** execute the child's tasks — those
     *       still require participants to act via the standard command
     *       interface. The client calls advance() after a task completes
     *       (to dispatch a newly-reachable sub-process) and after a child
     *       instance completes (to let the parent resume).
     * 
     * @note **Ownership.** The WorkflowEngine takes sole ownership of the
     *       nested WorkflowInstance and stores it in its instances map.
     *       The SubProcessInstance holds only a non-owning pointer.
     * 
     * @param instanceId The unique ID of the parent workflow instance to advance.
     * 
     * @throws Nothing. If the repository is null or the definition is missing,
     *         the method silently returns without dispatching.
     */
    void advance(const std::string& instanceId); 

};
#endif
