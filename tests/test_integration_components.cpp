/**
 * @file test_integration_components.cpp
 * @brief Integration tests focusing on pairwise/triple class interactions.
 */
#include <gtest/gtest.h>
#include <memory>

#include "ActivityDefinition.h"
#include "StageDefinition.h"
#include "WorkItem.h"
#include "StageInstance.h"
#include "Participant.h"
#include "WorkflowEngine.h"
#include "EventBus.h"
#include "HistoryManager.h"
#include "AuditObserver.h"
#include "AssignCommand.h"
#include "StartCommand.h"
#include "InstanceFactory.h"
#include "RoleBasedAssignment.h"
#include "WorkloadBasedAssignment.h"
#include "CompleteCommand.h"
#include "makeAvailableCommand.h"

// 1. Engine + HistoryManager (Command + Caretaker)
TEST(IntegrationComponents, EngineAndHistoryUndo) {
    ActivityDefinition def("task", "Task", "worker");
    WorkItem item("inst-1", &def);
    Participant alice("p1", "Alice", {"worker"});

    EventBus bus;
    HistoryManager history;
    WorkflowEngine engine(nullptr, &bus, &history, nullptr);

    makeAvailableCommand(&item).execute();
    engine.submit(new AssignCommand(&item, &alice));
    engine.submit(new StartCommand(&item));
    engine.submit(new CompleteCommand(&item));

    EXPECT_EQ(item.getState(), "Completed");
    EXPECT_EQ(history.getAll().size(), 3); // Assign, Start, Complete

    // Undo the completion
    engine.undoLast();
    EXPECT_EQ(item.getState(), "In_Progress");
    EXPECT_EQ(history.getAll().size(), 2);
}

//2. Engine + Observer (Command + Observer) 
TEST(IntegrationComponents, EngineAndObserverEventBus) {
    ActivityDefinition def("task", "Task", "worker");
    WorkItem item("inst-1", &def);
    Participant alice("p1", "Alice", {"worker"});

    EventBus bus;
    AuditObserver audit;
    bus.attach(&audit);

    WorkflowEngine engine(nullptr, &bus, nullptr, &audit);

    makeAvailableCommand(&item).execute();
    item.setEventBus(&bus); 
    engine.submit(new AssignCommand(&item, &alice));
    engine.submit(new StartCommand(&item));

    // The observer should have captured the state changes
    ASSERT_GT(audit.getLog().size(), 0);
    
    // Verify the log contains the expected state transition
    bool foundTransition = false;
    for (const std::string& line : audit.getLog()) {
        if (line.find("Assigned -> In_Progress") != std::string::npos) {
            foundTransition = true;
            break;
        }
    }
    EXPECT_TRUE(foundTransition);
}

//  3. InstanceFactory + Strategy (Factory + Strategy)
// Assignment is a state change, so the engine does it with commands after the factory has built the tree.
TEST(IntegrationComponents, FactoryLeavesAssignmentToTheEngine) {
    // 1. Build a Definition Tree with two activities
    StageDefinition root("root", "Root Stage");
    root.addChild(new ActivityDefinition("taskA", "Task A", "roleA"));
    root.addChild(new ActivityDefinition("taskB", "Task B", "roleB"));

    // 2. Create Participants
    Participant p1("id1", "Alice", {"roleA"});
    Participant p2("id2", "Bob",   {"roleB"});
    std::vector<Participant*> candidates = {&p1, &p2};

    // 3. Setup the Strategy
    RoleBasedAssignment strategy;

    // 4. Factory builds the tree
    std::unique_ptr<RuntimeComponent> tree(
        InstanceFactory::instantiateTree("inst-1", &root, &strategy, candidates)
    );

    ASSERT_NE(tree, nullptr);
    auto* stage = dynamic_cast<StageInstance*>(tree.get());
    ASSERT_NE(stage, nullptr);
    ASSERT_EQ(stage->getChildren().size(), 2);

    // 5. Nothing is assigned yet
    auto* taskA = dynamic_cast<WorkItem*>(stage->getChildren()[0]);
    auto* taskB = dynamic_cast<WorkItem*>(stage->getChildren()[1]);

    ASSERT_NE(taskA, nullptr);
    ASSERT_NE(taskB, nullptr);

    EXPECT_EQ(taskA->getAssignedParticipant(), nullptr);
    EXPECT_EQ(taskB->getAssignedParticipant(), nullptr);
}