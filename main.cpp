
// main.cpp
#include <iostream>
#include "WorkItem.h"
#include "ActivityDefinition.h"
#include "CreatedState.h"
#include "AvailableState.h"
#include "AssignedState.h"
#include "InProgressState.h"
#include "CompletedState.h"
#include "RejectedState.h"
#include "CancelledState.h"
#include "EscalatedState.h"
#include "FailedState.h"
#include "Participant.h"

// --- Tiny test helpers -------------------------------------------------
static int passed = 0, failed = 0;

void expect(bool cond, const std::string& what) {
    if (cond) { ++passed; std::cout << "  [PASS] " << what << "\n"; }
    else      { ++failed; std::cout << "  [FAIL] " << what << "\n"; }
}

// Identify the dynamic type of the current state via typeid
std::string stateName(WorkItem* w) {
    WorkItemState* s = w->getCurrentState(); // add accessor, or use a friend
    if (!s) return "<null>";
    return typeid(*s).name();
}

// --- Fake ActivityDefinition (minimal) --------------------------------
// Adjust if your real ActivityDefinition differs.
class TestActivity : public ActivityDefinition {
public:
    std::string getId()   const override { return "act-1"; }
    std::string getName() const override { return "TestActivity"; }
};

int main() {
    std::cout << "===== Happy path: Created -> Available -> Assigned -> InProgress -> Completed =====\n";
    {
        WorkItem wi("wi-1", new TestActivity());
        Participant p;

        // Created state
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("CreatedState") != std::string::npos,
               "initial state is CreatedState");

        wi.makeAvailable();
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("AvailableState") != std::string::npos,
               "Created -> Available");

        wi.assign(&p);
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("AssignedState") != std::string::npos,
               "Available -> Assigned");

        wi.start();
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("InProgressState") != std::string::npos,
               "Assigned -> InProgress");

        wi.complete();
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("CompletedState") != std::string::npos,
               "InProgress -> Completed");
    }

    std::cout << "\n===== Cancel paths =====\n";
    {
        WorkItem wi("wi-2", new TestActivity());
        wi.cancel(); // Created -> Cancelled
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("CancelledState") != std::string::npos,
               "Created -> Cancelled");
    }
    {
        WorkItem wi("wi-3", new TestActivity());
        wi.makeAvailable();
        wi.cancel(); // Available -> Cancelled
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("CancelledState") != std::string::npos,
               "Available -> Cancelled");
    }
    {
        WorkItem wi("wi-4", new TestActivity());
        wi.makeAvailable();
        Participant p; wi.assign(&p);
        wi.cancel(); // Assigned -> Cancelled
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("CancelledState") != std::string::npos,
               "Assigned -> Cancelled");
    }
    {
        WorkItem wi("wi-5", new TestActivity());
        wi.makeAvailable(); Participant p; wi.assign(&p);
        wi.start();
        wi.cancel(); // InProgress -> Cancelled
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("CancelledState") != std::string::npos,
               "InProgress -> Cancelled");
    }

    std::cout << "\n===== Reject path =====\n";
    {
        WorkItem wi("wi-6", new TestActivity());
        wi.makeAvailable(); Participant p; wi.assign(&p);
        wi.reject(); // Assigned -> Rejected
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("RejectedState") != std::string::npos,
               "Assigned -> Rejected");
        wi.makeAvailable(); // Rejected -> Available
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("AvailableState") != std::string::npos,
               "Rejected -> Available");
    }

    std::cout << "\n===== Escalate + Fail paths =====\n";
    {
        WorkItem wi("wi-7", new TestActivity());
        wi.makeAvailable(); Participant p; wi.assign(&p);
        wi.start();
        wi.escalated(); // InProgress -> Escalated
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("EscalatedState") != std::string::npos,
               "InProgress -> Escalated");

        wi.cancel(); // Escalated -> Cancelled
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("CancelledState") != std::string::npos,
               "Escalated -> Cancelled");
    }
    {
        WorkItem wi("wi-8", new TestActivity());
        wi.makeAvailable(); Participant p; wi.assign(&p);
        wi.start();
        wi.fail(); // InProgress -> Failed
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("FailedState") != std::string::npos,
               "InProgress -> Failed");

        wi.makeAvailable(); // Failed -> Available
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("AvailableState") != std::string::npos,
               "Failed -> Available");
    }

    std::cout << "\n===== Invalid transitions (should print 'Invalid State Transition') =====\n";
    {
        WorkItem wi("wi-9", new TestActivity());
        // Created state: start/complete/reject/escalated/fail are invalid
        std::cout << "  -- CreatedState invalid calls --\n";
        wi.start();
        wi.complete();
        wi.reject();
        wi.escalated();
        wi.fail();

        // Still CreatedState after all invalid calls
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("CreatedState") != std::string::npos,
               "state unchanged after invalid calls");
    }
    {
        WorkItem wi("wi-10", new TestActivity());
        wi.makeAvailable();
        // AvailableState: start/complete/reject/fail invalid
        std::cout << "  -- AvailableState invalid calls --\n";
        wi.start();
        wi.complete();
        wi.reject();
        wi.fail();
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("AvailableState") != std::string::npos,
               "AvailableState unchanged after invalid calls");
    }
    {
        WorkItem wi("wi-11", new TestActivity());
        wi.makeAvailable(); Participant p; wi.assign(&p);
        wi.start(); wi.complete(); // now Completed
        std::cout << "  -- CompletedState invalid calls --\n";
        wi.cancel();
        wi.escalated();
        wi.makeAvailable();
        expect(std::string(typeid(*wi.getCurrentState()).name()).find("CompletedState") != std::string::npos,
               "CompletedState unchanged after invalid calls");
    }

    std::cout << "\n=============================================\n";
    std::cout << "Passed: " << passed << "  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}

/*#include <iostream>
#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "ConcreteWorkflowBuilder.h"
#include "InstanceFactory.h"
#include "WorkflowDefinition.h"

int main() {
    std::cout << " TIVIDY: BUILDER + COMPOSITE + INSTANCE FACTORY INTEGRATION TEST\n";

    WorkflowDirector director;
    WorkflowBlueprint blueprint;
    blueprint.id = "project-approval";
    blueprint.name = "Tech Company Project Approval";
    blueprint.version = 1;

    std::cout << "--- PHASE 1: USER PROVIDES RAW DATA---\n";
    blueprint.roles.push_back(RoleDefinition("proposer", "Proposer", 0));
    blueprint.roles.push_back(RoleDefinition("team_lead", "Team Lead", 1));

    // The user constructs a raw data tree
    WorkflowNodeData rootData;
    rootData.type = NodeType::STAGE;
    rootData.id = "root";
    rootData.name = "Main Process";

    // Leaf 1: Submit Activity
    WorkflowNodeData submitData;
    submitData.type = NodeType::ACTIVITY;
    submitData.id = "submit";
    submitData.name = "Submit Proposal";
    submitData.roleId = "proposer";
    rootData.children.push_back(submitData);

    // Branch 1: Review Stage
    WorkflowNodeData reviewData;
    reviewData.type = NodeType::STAGE;
    reviewData.id = "review";
    reviewData.name = "Review Phase";
    
    WorkflowNodeData techReviewData;
    techReviewData.type = NodeType::ACTIVITY;
    techReviewData.id = "techReview";
    techReviewData.name = "Technical Review";
    techReviewData.roleId = "team_lead";
    reviewData.children.push_back(techReviewData);
    
    rootData.children.push_back(reviewData);

    // Branch 2: SubProcess
    WorkflowNodeData paymentData;
    paymentData.type = NodeType::SUBPROCESS;
    paymentData.id = "payment";
    paymentData.calledDefId = "payment-process";
    paymentData.calledVersion = 1;
    rootData.children.push_back(paymentData);

    // Give the raw tree to the blueprint
    blueprint.rootTreeData = rootData;
    blueprint.hasRootTree = true;
    std::cout << "User blueprint populated with raw data nodes.\n\n";


    std::cout << "--- PHASE 2: BUILDER PATTERN (Director & Concrete Builder) ---\n";
    std::cout << "Passing blueprint to Director and Concrete Builder...\n";
    ConcreteWorkflowBuilder builder;
    director.construct(builder, blueprint);
    
    // The Builder recursively creates the actual C++ objects here
    WorkflowDefinition def = builder.build(); 
    std::cout << "Successfully built WorkflowDefinition: " << def.getName() << "\n\n";


    std::cout << "--- PHASE 3: DEFINITION COMPOSITE TREE (The Static Blueprint) ---\n";
    std::cout << "This tree represents every possible path (immutable, read-only).\n";
    if (def.getRoot() != nullptr) {
        def.getRoot()->print();
    }
    std::cout << "\n";


    std::cout << "--- PHASE 4: INSTANTIATION (Creating the Runtime Tree) ---\n";
    std::string instanceId = "Claim-C-1001";
    std::cout << "Creating a running instance for ID: " << instanceId << "...\n";
    std::cout << "The InstanceFactory traverses the Definition Tree and generates\n";
    std::cout << "a Runtime Tree with mutable WorkItems and StageInstances.\n\n";

    RuntimeComponent* runtimeRoot = InstanceFactory::instantiateTree(instanceId, def.getRoot());
    
    std::cout << "Live Runtime Tree (Notice all states start as IDLE):\n";
    runtimeRoot->print();
    std::cout << "\n";


    std::cout << "--- PHASE 5: RUNTIME EXECUTION (Simulating Work) ---\n";
    std::cout << "Simulating a user completing the 'Submit Proposal' activity...\n";
    
    // Traverse the runtime tree and change the state of the 'submit' item
    auto children = runtimeRoot->getChildren();
    if (!children.empty()) {
        // Find the 'submit' activity specifically
        for (auto* child : children) {
            if (child->getId() == "submit") {
                child->setState("SUCCESS");
                std::cout << "-> State of '" << child->getId() << "' changed to SUCCESS.\n";
                break;
            }
        }
    }

    std::cout << "\nUpdated Runtime Tree:\n";
    runtimeRoot->print();

    std::cout << "\n=================================================================\n";
    std::cout << " TEST COMPLETE.\n";
    std::cout << " This proves the Definition Tree is read-only, while the Runtime\n";
    std::cout << " Tree is mutable and tracks the specific state of one instance.\n";
    std::cout << "=================================================================\n";

    // Clean up memory
    delete runtimeRoot; // Recursively deletes the runtime tree
    // The 'def' object goes out of scope here, deleting the Definition Tree.

    return 0;
}*/