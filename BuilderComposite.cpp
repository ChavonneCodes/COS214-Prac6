

#include <iostream>
#include <string>
#include <vector>
#include <map>

// Builder + definition side
#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "ConcreteWorkflowBuilder.h"
#include "WorkflowDefinition.h"
#include "WorkflowComponent.h"
#include "ActivityDefinition.h"
#include "StageDefinition.h"
#include "SubProcessDefinition.h"

// Runtime side
#include "RuntimeComponent.h"
#include "WorkItem.h"
#include "StageInstance.h"
#include "SubProcessInstance.h"
#include "WorkflowInstance.h"
#include "InstanceFactory.h"

// Engine + services
#include "InMemoryDefinitionRepository.h"
#include "WorkflowEngine.h"
#include "EventBus.h"
#include "AuditObserver.h"
#include "HistoryManager.h"

// Strategies + commands
#include "Participant.h"
#include "RoleBasedAssignment.h"
#include "SequentialRouting.h"
#include "RoutingHelper.h"
#include "AssignCommand.h"
#include "StartCommand.h"
#include "CompleteCommand.h"

// ============================================================================
// Helpers
// ============================================================================

static void heading(const std::string& title) {
    std::cout << "\n=================================================================\n"
              << " " << title << "\n"
              << "=================================================================\n";
}

/** Builds a WorkflowNodeData for an activity. */
static WorkflowNodeData activity(const std::string& id,
                                 const std::string& name,
                                 const std::string& roleId) {
    WorkflowNodeData n;
    n.type   = NodeType::ACTIVITY;
    n.id     = id;
    n.name   = name;
    n.roleId = roleId;
    return n;
}

/** Builds a WorkflowNodeData for a stage. */
static WorkflowNodeData stage(const std::string& id, const std::string& name) {
    WorkflowNodeData n;
    n.type = NodeType::STAGE;
    n.id   = id;
    n.name = name;
    return n;
}

/** Builds a WorkflowNodeData for a sub-process call. */
static WorkflowNodeData subProcess(const std::string& id,
                                   const std::string& calledDefId,
                                   int calledVersion) {
    WorkflowNodeData n;
    n.type          = NodeType::SUBPROCESS;
    n.id            = id;
    n.name          = "Call " + calledDefId;
    n.calledDefId   = calledDefId;
    n.calledVersion = calledVersion;
    return n;
}

/** Recursively finds a runtime node by id. */
static RuntimeComponent* findRuntime(RuntimeComponent* root, const std::string& id) {
    if (!root) return nullptr;
    if (root->getId() == id) return root;
    for (RuntimeComponent* c : root->getChildren()) {
        RuntimeComponent* f = findRuntime(c, id);
        if (f) return f;
    }
    return nullptr;
}

static void processTask(WorkflowEngine& engine, WorkItem* item) {
    if (!item) return;
    std::cout << "  > Processing " << item->getId() << "\n";
    engine.submit(new StartCommand(item));       // Assigned → InProgress
    engine.submit(new CompleteCommand(item));    // InProgress → Completed
}

// ============================================================================
// BLUEPRINTS
// ============================================================================

/**
 * @brief CHILD workflow: payment-processing
 *
 * Tree:
 *   [Stage] Payment
 *     ├── Activity: Verify Bank
 *     ├── Activity: Transfer Funds
 *     └── Activity: Send Receipt
 */
static WorkflowBlueprint buildChildBlueprint() {
    WorkflowBlueprint bp;
    bp.id      = "payment-processing";
    bp.name    = "Payment Processing";
    bp.version = 1;

    bp.roles.push_back(RoleDefinition("payment_agent", "Payment Agent", 1));

    bp.activities.push_back(ActivityDefinition("verifyBank", "Verify Bank",    "payment_agent"));
    bp.activities.push_back(ActivityDefinition("transfer",   "Transfer Funds", "payment_agent"));
    bp.activities.push_back(ActivityDefinition("receipt",    "Send Receipt",   "payment_agent"));

    bp.transitions.push_back(TransitionDefinition("verifyBank", "transfer", "always"));
    bp.transitions.push_back(TransitionDefinition("transfer",   "receipt",  "always"));

    WorkflowNodeData root = stage("paymentRoot", "Payment");
    root.children.push_back(activity("verifyBank", "Verify Bank",    "payment_agent"));
    root.children.push_back(activity("transfer",   "Transfer Funds", "payment_agent"));
    root.children.push_back(activity("receipt",    "Send Receipt",   "payment_agent"));

    bp.rootTreeData = root;
    bp.hasRootTree  = true;
    return bp;
}

/**
 * @brief PARENT workflow: project-approval
 *
 * Tree:
 *   [Stage] Project Approval
 *     ├── Activity: Submit Proposal
 *     ├── [Stage] Review
 *     │     ├── Activity: Product Review
 *     │     └── Activity: Technical Review
 *     ├── SubProcess: Call payment-processing v1
 *     └── Activity: Project Kickoff
 */
static WorkflowBlueprint buildParentBlueprint() {
    WorkflowBlueprint bp;
    bp.id      = "project-approval";
    bp.name    = "Project Approval";
    bp.version = 1;

    bp.roles.push_back(RoleDefinition("proposer", "Proposer", 0));
    bp.roles.push_back(RoleDefinition("reviewer", "Reviewer", 1));

    bp.activities.push_back(ActivityDefinition("submit",  "Submit Proposal",  "proposer"));
    bp.activities.push_back(ActivityDefinition("product", "Product Review",   "reviewer"));
    bp.activities.push_back(ActivityDefinition("tech",    "Technical Review", "reviewer"));
    bp.activities.push_back(ActivityDefinition("kickoff", "Project Kickoff",  "proposer"));

    // Transitions — note the "sub-process" step between tech and kickoff
    bp.transitions.push_back(TransitionDefinition("submit",  "product", "always"));
    bp.transitions.push_back(TransitionDefinition("product", "tech",    "always"));
    bp.transitions.push_back(TransitionDefinition("tech",    "payment", "always"));
    bp.transitions.push_back(TransitionDefinition("payment", "kickoff", "always"));

    // Composite tree
    WorkflowNodeData root = stage("projectRoot", "Project Approval");
    root.children.push_back(activity("submit", "Submit Proposal", "proposer"));

    // Nested stage
    WorkflowNodeData review = stage("reviewStage", "Review");
    review.children.push_back(activity("product", "Product Review",   "reviewer"));
    review.children.push_back(activity("tech",    "Technical Review", "reviewer"));
    root.children.push_back(review);

    // Sub-process node
    root.children.push_back(subProcess("payment", "payment-processing", 1));

    root.children.push_back(activity("kickoff", "Project Kickoff", "proposer"));

    bp.rootTreeData = root;
    bp.hasRootTree  = true;
    return bp;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    heading("TIVIDY: STAGES + ACTIVITIES + SUB-PROCESSES + BUILDER + ROUTING");

    // ------------------------------------------------------------------------
    heading("PHASE 1: Setup");
    // ------------------------------------------------------------------------
    InMemoryDefinitionRepository repository;
    EventBus eventBus;
    AuditObserver audit;
    eventBus.attach(&audit);
    HistoryManager history;
    WorkflowEngine engine(&repository, &eventBus, &history, &audit);

    Participant deon  ("p1", "Deon",   {"proposer"});
    Participant lerato("p2", "Lerato", {"reviewer"});
    Participant ayesha("p3", "Ayesha", {"payment_agent"});
    std::vector<Participant*> staff = {&deon, &lerato, &ayesha};
    RoleBasedAssignment assignment;
    engine.setAssignment(&assignment, staff);

    // ------------------------------------------------------------------------
    heading("PHASE 2: Builder builds CHILD definition");
    // ------------------------------------------------------------------------
    WorkflowDirector director;
    {
        ConcreteWorkflowBuilder builder;
        WorkflowBlueprint bp = buildChildBlueprint();
        director.construct(builder, bp);
        WorkflowDefinition* def = new WorkflowDefinition(builder.build());
        repository.save(def);
        std::cout << "Built & stored: '" << def->getDefinitionId()
                  << "' v" << def->getVersion() << "\n";
    }

    // ------------------------------------------------------------------------
    heading("PHASE 3: Builder builds PARENT definition (with sub-process)");
    // ------------------------------------------------------------------------
    {
        ConcreteWorkflowBuilder builder;
        WorkflowBlueprint bp = buildParentBlueprint();
        director.construct(builder, bp);
        WorkflowDefinition* def = new WorkflowDefinition(builder.build());
        repository.save(def);
        std::cout << "Built & stored: '" << def->getDefinitionId()
                  << "' v" << def->getVersion() << "\n";
    }

    // ------------------------------------------------------------------------
    heading("PHASE 4: Definition Composite trees (what the Builder produced)");
    // ------------------------------------------------------------------------
    WorkflowDefinition* parentDef = repository.load("project-approval", 1);
    WorkflowDefinition* childDef  = repository.load("payment-processing", 1);

    std::cout << "\n[PARENT definition tree]\n";
    parentDef->getRoot()->print(0);

    std::cout << "\n[CHILD definition tree]\n";
    childDef->getRoot()->print(0);

    // ------------------------------------------------------------------------
    heading("PHASE 5: Start parent workflow (Factory instantiates the tree)");
    // ------------------------------------------------------------------------
    std::string parentId = engine.startWorkflow("project-approval", 1);
    WorkflowInstance* parent = engine.getInstance(parentId);

    std::cout << "Parent instance: " << parentId << "\n";
    std::cout << "\nRuntime tree (all work items start IDLE, sub-process starts IDLE):\n";
    parent->getRoot()->print(1);

    // ------------------------------------------------------------------------
    heading("PHASE 6: Route through the parent until the sub-process");
    // ------------------------------------------------------------------------
    // Step 1: submit
    std::cout << "\n[submit]\n";
    processTask(engine, parent->findWorkItem("submit"));

    // Step 2: product review
    std::cout << "\n[product]\n";
    processTask(engine, parent->findWorkItem("product"));

    // Step 3: technical review
    std::cout << "\n[tech]\n";
    processTask(engine, parent->findWorkItem("tech"));

    // Route from "tech"
    SequentialRouting sequential;
    auto* techDef = findNode(*parentDef, "tech");
    std::vector<const WorkflowComponent*> next = sequential.next(*techDef, *parentDef, "approved");

    std::cout << "\n[ROUTING] After 'tech', SequentialRouting says next is: ";
    if (!next.empty()) std::cout << next[0]->getId() << "\n";
    else std::cout << "(none)\n";

    // Verify the next node is a SubProcessDefinition
    if (!next.empty() && dynamic_cast<const SubProcessDefinition*>(next[0])) {
        std::cout << "[ROUTING] ✅ Next node is a SUB-PROCESS — needs dispatch.\n";
    }

    // ------------------------------------------------------------------------
    heading("PHASE 7: Dispatch the sub-process via engine.advance()");
    // ------------------------------------------------------------------------
    engine.advance(parentId);

    // Grab the child instance that advance() created
    std::string childId = parentId + "-payment";
    WorkflowInstance* child = engine.getInstance(childId);

    std::cout << "\nParent tree after dispatch (child is nested inside 'payment'):\n";
    parent->getRoot()->print(1);

    if (child) {
        std::cout << "\nChild runtime tree:\n";
        child->getRoot()->print(1);

        // ---------------------------------------------------------------------
        heading("PHASE 8: Run the child workflow to completion");
        // ---------------------------------------------------------------------
        processTask(engine, child->findWorkItem("verifyBank"));
        processTask(engine, child->findWorkItem("transfer"));
        processTask(engine, child->findWorkItem("receipt"));

        child->setState("COMPLETED");
        std::cout << "\nChild instance state: " << child->getState() << "\n";
    }

    // ------------------------------------------------------------------------
    heading("PHASE 9: Resume the parent via engine.advance()");
    // ------------------------------------------------------------------------
    engine.advance(parentId);

    auto* subRuntime = dynamic_cast<SubProcessInstance*>(
        findRuntime(parent->getRoot(), "payment"));
    if (subRuntime) {
        std::cout << "Sub-process state after child completed: "
                  << subRuntime->getState() << "\n";
    }

    // Route from "payment" to the next parent node
    auto* paymentDef = findNode(*parentDef, "payment");
    std::vector<const WorkflowComponent*> afterPayment =
        sequential.next(*paymentDef, *parentDef, "approved");

    std::cout << "[ROUTING] After 'payment', next is: ";
    if (!afterPayment.empty()) std::cout << afterPayment[0]->getId() << "\n";
    else std::cout << "(none)\n";

    // ------------------------------------------------------------------------
    heading("PHASE 10: Finish the parent workflow");
    // ------------------------------------------------------------------------
    processTask(engine, parent->findWorkItem("kickoff"));
    parent->setState("COMPLETED");

    std::cout << "\nFinal parent runtime tree:\n";
    parent->getRoot()->print(1);

    std::cout << "\nParent state: " << parent->getState() << "\n";
    if (child) std::cout << "Child  state: " << child->getState() << "\n";

    // ------------------------------------------------------------------------
    heading("PHASE 11: Audit trail (Observer)");
    // ------------------------------------------------------------------------
    std::cout << "Audit captured " << audit.getLog().size() << " events.\n\n";
    for (const auto& line : audit.getLog()) {
        std::cout << "  " << line << "\n";
    }

    heading("DEMO COMPLETE");
    return 0;
}