#include <iostream>
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
}