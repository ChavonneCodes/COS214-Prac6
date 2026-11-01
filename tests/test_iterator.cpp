#include "../StageInstance.h"
#include "../RuntimeComponent.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>

class IteratorTest : public ::testing::Test {
protected:
    StageInstance* stageInstanceRoot;

    void SetUp() override {
        // Build the tree structure with IDs matching their names
        stageInstanceRoot = new StageInstance("root", new StageDefinition("root", "root"));
        StageInstance* stageInstance0 = new StageInstance("name0", new StageDefinition("name0", "name0"));
        StageInstance* stageInstance01 = new StageInstance("name01", new StageDefinition("name01", "name01"));
        StageInstance* stageInstance02 = new StageInstance("name02", new StageDefinition("name02", "name02"));
        StageInstance* stageInstance1 = new StageInstance("name1", new StageDefinition("name1", "name1"));
        StageInstance* stageInstance2 = new StageInstance("name2", new StageDefinition("name2", "name2"));

        stageInstanceRoot->addChild(stageInstance0);
        stageInstance0->addChild(stageInstance01);
        stageInstance0->addChild(stageInstance02);
        stageInstanceRoot->addChild(stageInstance1);
        stageInstanceRoot->addChild(stageInstance2);
    }

    void TearDown() override {
        // Clean up to prevent memory leaks during test runs
        delete stageInstanceRoot; 
    }
};

TEST_F(IteratorTest, DFSReturnsCorrectOrder) {
    auto iterDFS = stageInstanceRoot->createIterator(new DFSTraversalStrategy<RuntimeComponent>());
    
    std::vector<std::string> actualOrder;
    while(iterDFS->hasNext()) {
        const auto* comp = dynamic_cast<const RuntimeComponent*>(&iterDFS->getNext());
        actualOrder.push_back(comp->getId()); 
    }

    // Expected Depth-First Search order
    std::vector<std::string> expectedOrder = {
        "name0", "name01", "name02", "name1", "name2"
    };

    EXPECT_EQ(actualOrder, expectedOrder);
}

TEST_F(IteratorTest, BFSReturnsCorrectOrder) {
    auto iterBFS = stageInstanceRoot->createIterator(new BFSTraversalStrategy<RuntimeComponent>());
    
    std::vector<std::string> actualOrder;
    while(iterBFS->hasNext()) {
        const auto* comp = dynamic_cast<const RuntimeComponent*>(&iterBFS->getNext());
        actualOrder.push_back(comp->getId()); 
    }

    // Expected Breadth-First Search (level) order
    std::vector<std::string> expectedOrder = 
	    { "name2", "name1", "name0", "name02", "name01" };

    EXPECT_EQ(actualOrder, expectedOrder);
}
