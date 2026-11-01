
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "Iterator.h"
#include "StageInstance.h"
#include "StageDefinition.h"

class IteratorTest : public ::testing::Test {
protected:
    StageInstance* stageInstanceRoot;

    // StageDefinition objects are not owned by StageInstance.
    std::vector<StageDefinition*> definitions;

    StageDefinition* createDefinition(
        const std::string& id,
        const std::string& name
    ) {
        StageDefinition* definition = new StageDefinition(id, name);
        definitions.push_back(definition);
        return definition;
    }

    void SetUp() override {
        stageInstanceRoot = new StageInstance(
            "id",
            createDefinition("id", "root")
        );

        StageInstance* stageInstance0 = new StageInstance(
            "id",
            createDefinition("id", "name0")
        );

        StageInstance* stageInstance01 = new StageInstance(
            "id",
            createDefinition("id", "name01")
        );

        StageInstance* stageInstance02 = new StageInstance(
            "id",
            createDefinition("id", "name02")
        );

        StageInstance* stageInstance1 = new StageInstance(
            "id",
            createDefinition("id", "name1")
        );

        StageInstance* stageInstance2 = new StageInstance(
            "id",
            createDefinition("id", "name2")
        );

        stageInstanceRoot->addChild(stageInstance0);

        stageInstance0->addChild(stageInstance01);
        stageInstance0->addChild(stageInstance02);

        stageInstanceRoot->addChild(stageInstance1);
        stageInstanceRoot->addChild(stageInstance2);
    }

    void TearDown() override {
        // Root recursively deletes all its runtime children.
        delete stageInstanceRoot;

        // Definitions must be deleted separately.
        for (StageDefinition* definition : definitions) {
            delete definition;
        }

        definitions.clear();
    }

    std::vector<std::string> collectTraversal(
        Iterator<RuntimeComponent>* iterator
    ) {
        std::vector<std::string> result;

        while (iterator->hasNext()) {
            const Iterable<RuntimeComponent>& next = iterator->getNext();

            const RuntimeComponent* component =
                dynamic_cast<const RuntimeComponent*>(&next);

            ASSERT_NE(component, nullptr);

            // RuntimeComponent must provide a getName() method
            // for this helper to identify the traversed nodes.
            result.push_back(component->getName());
        }

        delete iterator;
        return result;
    }
};

// Test 1: DFS visits all nodes.
TEST_F(IteratorTest, DepthFirstTraversalVisitsAllNodes) {
    Iterator<RuntimeComponent>* iterator =
        stageInstanceRoot->createIterator(
            new DFSTraversalStrategy<RuntimeComponent>()
        );

    std::vector<std::string> result = collectTraversal(iterator);

    EXPECT_EQ(result.size(), 6);
}

// Test 2: BFS visits all nodes.
TEST_F(IteratorTest, BreadthFirstTraversalVisitsAllNodes) {
    Iterator<RuntimeComponent>* iterator =
        stageInstanceRoot->createIterator(
            new BFSTraversalStrategy<RuntimeComponent>()
        );

    std::vector<std::string> result = collectTraversal(iterator);

    EXPECT_EQ(result.size(), 6);
}

// Test 3: DFS follows depth-first order.
TEST_F(IteratorTest, DepthFirstTraversalUsesCorrectOrder) {
    Iterator<RuntimeComponent>* iterator =
        stageInstanceRoot->createIterator(
            new DFSTraversalStrategy<RuntimeComponent>()
        );

    std::vector<std::string> result = collectTraversal(iterator);

    std::vector<std::string> expected = {
        "root",
        "name0",
        "name01",
        "name02",
        "name1",
        "name2"
    };

    EXPECT_EQ(result, expected);
}

// Test 4: BFS follows breadth-first order.
TEST_F(IteratorTest, BreadthFirstTraversalUsesCorrectOrder) {
    Iterator<RuntimeComponent>* iterator =
        stageInstanceRoot->createIterator(
            new BFSTraversalStrategy<RuntimeComponent>()
        );

    std::vector<std::string> result = collectTraversal(iterator);

    std::vector<std::string> expected = {
        "root",
        "name0",
        "name1",
        "name2",
        "name01",
        "name02"
    };

    EXPECT_EQ(result, expected);
}

// Test 5: Iterator correctly reports when traversal is complete.
TEST_F(IteratorTest, IteratorReportsWhenTraversalIsComplete) {
    Iterator<RuntimeComponent>* iterator =
        stageInstanceRoot->createIterator(
            new DFSTraversalStrategy<RuntimeComponent>()
        );

    for (int i = 0; i < 6; ++i) {
        ASSERT_TRUE(iterator->hasNext());
        iterator->getNext();
    }

    EXPECT_FALSE(iterator->hasNext());

    delete iterator;
}

// Test 6: DFS and BFS produce different traversal orders.
TEST_F(IteratorTest, TraversalStrategiesProduceDifferentOrders) {
    Iterator<RuntimeComponent>* dfsIterator =
        stageInstanceRoot->createIterator(
            new DFSTraversalStrategy<RuntimeComponent>()
        );

    Iterator<RuntimeComponent>* bfsIterator =
        stageInstanceRoot->createIterator(
            new BFSTraversalStrategy<RuntimeComponent>()
        );

    std::vector<std::string> dfsResult =
        collectTraversal(dfsIterator);

    std::vector<std::string> bfsResult =
        collectTraversal(bfsIterator);

    EXPECT_NE(dfsResult, bfsResult);
}
