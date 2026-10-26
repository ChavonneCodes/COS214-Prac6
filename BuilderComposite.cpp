#include <iostream>
#include <vector>

// Definition includes
#include "WorkflowBlueprint.h"
#include "WorkflowDirector.h"
#include "ConcreteWorkflowBuilder.h"
#include "WorkflowDefinition.h"
#include "InMemoryDefinitionRepository.h"
#include "RoutingHelper.h"

// Runtime includes
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"
#include "InstanceFactory.h"
#include "Participant.h"
#include "RoleBasedAssignment.h"
#include "SequentialRouting.h"
#include "ConditionalRouting.h"
#include "EventBus.h"
#include "AuditObserver.h"
#include "HistoryManager.h"

// Composite Runtime Includes
#include "StageDefinition.h"
#include "StageInstance.h"
#include "WorkItem.h"
#include "ActivityDefinition.h"

// Helper function to find a specific node in the Runtime Tree by its ID
RuntimeComponent* findRuntimeNode(RuntimeComponent* root, const std::string& id) {
    if (!root) return nullptr;
    if (root->getId() == id) return root;
    
    for (RuntimeComponent* child : root->getChildren()) {
        RuntimeComponent* found = findRuntimeNode(child, id);
        if (found) return found;
    }
    return nullptr;
}

int main() {
    std::cout << "==================================================\n";
    std::cout << " TIVIDY: COMPOSITE TREE + ROUTING TEST\n";
    std::cout << "==================================================\n\n";

    // 1. Setup Services
    InMemoryDefinitionRepository repository;
    EventBus eventBus;
    AuditObserver auditObserver;
    eventBus.attach(&auditObserver);
    HistoryManager historyManager; 
    WorkflowEngine engine(&repository, &eventBus, &historyManager, &auditObserver);

    // 2. Build a Definition Tree
    std::cout << "--- Building Definition Tree ---\n";
    WorkflowBlueprint blueprint;
    blueprint.id = "test-routing-tree";
    blueprint.name = "Nested Routing Test";
    
    // FIX 1: Add the activities to the flat list as well, so RoutingHelper can find them
    blueprint.activities.push_back(ActivityDefinition("a1", "Task 1", "user"));
    blueprint.activities.push_back(ActivityDefinition("a2", "Task 2", "user"));
    blueprint.activities.push_back(ActivityDefinition("a3", "Task 3", "user"));

    WorkflowNodeData rootData;
    rootData.type = NodeType::STAGE;
    rootData.id = "root";
    rootData.name = "Main Process";

    WorkflowNodeData t1;
    t1.type = NodeType::ACTIVITY;
    t1.id = "a1";
    t1.name = "Task 1";
    t1.roleId = "user";
    rootData.children.push_back(t1);

    WorkflowNodeData reviewStage;
    reviewStage.type = NodeType::STAGE;
    reviewStage.id = "review_stage";
    reviewStage.name = "Review Phase";

    WorkflowNodeData t2;
    t2.type = NodeType::ACTIVITY;
    t2.id = "a2";
    t2.name = "Task 2";
    t2.roleId = "user";
    reviewStage.children.push_back(t2);

    WorkflowNodeData t3;
    t3.type = NodeType::ACTIVITY;
    t3.id = "a3";
    t3.name = "Task 3";
    t3.roleId = "user";
    reviewStage.children.push_back(t3);

    rootData.children.push_back(reviewStage);

    blueprint.rootTreeData = rootData;
    blueprint.hasRootTree = true;

    blueprint.transitions.push_back(TransitionDefinition("a1", "a2", "always"));
    blueprint.transitions.push_back(TransitionDefinition("a2", "a3", "approved"));
    
    WorkflowDirector director;
    ConcreteWorkflowBuilder builder;
    director.construct(builder, blueprint);
    
    WorkflowDefinition* def = new WorkflowDefinition(builder.build());
    repository.save(def);

    // 3. Setup Strategy & Participants
    Participant alice("p1", "Alice", {"user"}, 0);
    std::vector<Participant*> candidates = {&alice};
    RoleBasedAssignment assignStrat;

    engine.setAssignment(&assignStrat, candidates);

    // 4. Start Workflow
    std::cout << "\n--- Starting Workflow ---\n";
    std::string instId = engine.startWorkflow("test-routing-tree", 1);
    WorkflowInstance* instance = engine.getInstance(instId);
    
    if (!instance) {
        std::cerr << "Failed to start workflow!\n";
        return 1;
    }

    std::cout << "\n--- Initial Runtime Tree (Built by Engine) ---\n";
    instance->print();

    // =========================================================================
    // 5. ROUTING IN ACTION
    // =========================================================================
    
    std::cout << "\n--- Executing Task 1 (Sequential Routing) ---\n";
    
    RuntimeComponent* rootRuntime = instance->getRoot();
    WorkItem* task1 = dynamic_cast<WorkItem*>(findRuntimeNode(rootRuntime, "a1"));
    
    if (task1) {
        // FIX 2: Must start before completing
        task1->start();
        task1->complete();
        
        const ActivityDefinition* t1Def = findActivity(*def, "a1");
        SequentialRouting seqRouter;
        
        std::vector<const ActivityDefinition*> nextDefs = seqRouter.next(*t1Def, *def, "success");
        if (!nextDefs.empty()) {
            std::cout << "-> Router determined next activity is: " << nextDefs[0]->getName() << "\n";
        }
    }

    std::cout << "\n--- Executing Task 2 (Conditional Routing) ---\n";
    
    WorkItem* task2 = dynamic_cast<WorkItem*>(findRuntimeNode(rootRuntime, "a2"));
    if (task2) {
        task2->start();
        task2->complete();
        
        const ActivityDefinition* t2Def = findActivity(*def, "a2");
        ConditionalRouting condRouter("approved", "a3", "");
        
        std::vector<const ActivityDefinition*> condNext = condRouter.next(*t2Def, *def, "approved");
        if (!condNext.empty()) {
            std::cout << "-> Router determined next activity is: " << condNext[0]->getName() << "\n";
        }
    }

    // 6. Final Output
    std::cout << "\n--- Final Runtime Tree (Task 1 & 2 Completed) ---\n";
    instance->print();

    return 0;
}