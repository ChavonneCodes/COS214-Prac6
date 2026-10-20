#include <iostream>
#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "ConcreteWorkflowBuilder.h"
#include "InstanceFactory.h"
#include "WorkflowDefinition.h"

// Include the strategies and Participant
#include "Participant.h"
#include "RoleBasedAssignment.h"
#include "WorkloadBasedAssignment.h"

int main() {
    std::cout << "=================================================================\n";
    std::cout << " TIVIDY: INTEGRATION TEST (BUILDER + COMPOSITE + STRATEGY)\n";
    std::cout << "=================================================================\n\n";

    WorkflowDirector director;
    WorkflowBlueprint blueprint;
    blueprint.id = "project-approval";
    blueprint.name = "Tech Company Project Approval";
    blueprint.version = 1;

    // =========================================================================
    std::cout << "--- PHASE 1: USER PROVIDES RAW DATA ---\n";
    // =========================================================================
    blueprint.roles.push_back(RoleDefinition("proposer", "Proposer", 0));
    blueprint.roles.push_back(RoleDefinition("team_lead", "Team Lead", 1));

    WorkflowNodeData rootData;
    rootData.type = NodeType::STAGE;
    rootData.id = "root";
    rootData.name = "Main Process";

    WorkflowNodeData submitData;
    submitData.type = NodeType::ACTIVITY;
    submitData.id = "submit";
    submitData.name = "Submit Proposal";
    submitData.roleId = "proposer";
    rootData.children.push_back(submitData);

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

    blueprint.rootTreeData = rootData;
    blueprint.hasRootTree = true;
    std::cout << "User blueprint populated with raw data nodes.\n\n";

    // =========================================================================
    std::cout << "--- PHASE 2: BUILDER PATTERN ---\n";
    // =========================================================================
    ConcreteWorkflowBuilder builder;
    director.construct(builder, blueprint);
    WorkflowDefinition def = builder.build(); 
    std::cout << "Successfully built WorkflowDefinition: " << def.getName() << "\n\n";

    // =========================================================================
    std::cout << "--- PHASE 3: DEFINITION COMPOSITE TREE (The Blueprint) ---\n";
    // =========================================================================
    std::cout << "This is the static, read-only tree. Notice there are no states or people.\n";
    if (def.getRoot() != nullptr) {
        def.getRoot()->print(); // <--- THIS PRINTS THE DEFINITION TREE
    } else {
        std::cout << "Error: Root is nullptr!\n";
    }
    std::cout << "\n";

    // =========================================================================
    std::cout << "--- PHASE 4: SETUP PARTICIPANTS & STRATEGY ---\n";
    // =========================================================================
    Participant alice("p1", "Alice", {"proposer"}, 0);
    Participant bob("p2", "Bob", {"team_lead"}, 1);
    Participant carol("p3", "Carol", {"team_lead"}, 2);

    std::vector<Participant*> candidates = {&alice, &bob, &carol};

    RoleBasedAssignment assignmentStrategy;
    std::cout << "Using Strategy: " << assignmentStrategy.getName() << "\n\n";

    // =========================================================================
    std::cout << "--- PHASE 5: INSTANTIATION (Creating the Runtime Tree) ---\n";
    // =========================================================================
    std::string instanceId = "Claim-C-1001";
    
    if (def.getRoot() == nullptr) {
        std::cerr << "ERROR: def.getRoot() is nullptr! The Composite tree was not built correctly.\n";
        return 1;
    }

    RuntimeComponent* runtimeRoot = InstanceFactory::instantiateTree(
        instanceId, def.getRoot(), &assignmentStrategy, candidates
    );
    
    if (runtimeRoot == nullptr) {
        std::cerr << "ERROR: runtimeRoot is nullptr! The InstanceFactory failed to build the runtime tree.\n";
        return 1;
    }
    
    std::cout << "\nLive Runtime Tree (Notice the states and assignments!):\n";
    runtimeRoot->print(); // <--- THIS PRINTS THE RUNTIME TREE
    std::cout << "\n";

    // =========================================================================
    std::cout << "--- PHASE 6: SIMULATING WORK ---\n";
    // =========================================================================
    std::cout << "Simulating a user completing the 'Submit Proposal' activity...\n";
    auto children = runtimeRoot->getChildren();
    if (!children.empty()) {
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
    std::cout << "=================================================================\n";

    // Clean up memory
    delete runtimeRoot;

    return 0;
}