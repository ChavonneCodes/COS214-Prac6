#include "../StageInstance.h"
#include "../RuntimeComponent.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>

class IteratorTest : public ::testing::Test {
protected:
    StageInstance* stageInstanceRoot;
    std::vector<StageDefinition*>definitions;

    /*void SetUp() override {
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

    }*/
    void SetUp() override {
    auto make = [this](const std::string& id) -> StageInstance* {
        auto* def = new StageDefinition(id, id);
        definitions.push_back(def);           // ← this is the key line
        return new StageInstance(id, def);
    };

    stageInstanceRoot       = make("root");
    StageInstance* name0    = make("name0");
    StageInstance* name01   = make("name01");
    StageInstance* name02   = make("name02");
    StageInstance* name1    = make("name1");
    StageInstance* name2    = make("name2");

    stageInstanceRoot->addChild(name0);
    name0->addChild(name01);
    name0->addChild(name02);
    stageInstanceRoot->addChild(name1);
    stageInstanceRoot->addChild(name2);
}

    /*void TearDown() override {
        // Clean up to prevent memory leaks during test runs
        delete stageInstanceRoot; 
        for (auto* def : definitions)
        delete def;
    }*/
   void TearDown() override {
    // Delete the StageInstance tree
    delete stageInstanceRoot;

    // Delete all StageDefinitions separately
    for (auto* def : definitions) {
        delete def;
    }

    definitions.clear();
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
    delete iterDFS; // Clean up the iterator to prevent memory leaks
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
    delete iterBFS; // Clean up the iterator to prevent memory leaks
}
