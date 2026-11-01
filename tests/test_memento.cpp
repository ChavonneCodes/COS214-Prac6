
#include <gtest/gtest.h>
#include <string>

#include "WorkItemMemento.h"
#include "Data.h"

// --------------------------------------------------
// Friend class for accessing private memento getters
// --------------------------------------------------

class WorkItemMementoTestAccess {
public:

    static std::string getActivityId(
        const WorkItemMemento& memento
    ) {
        return memento.getActivityId();
    }

    static std::string getWorkItemId(
        const WorkItemMemento& memento
    ) {
        return memento.getWorkItemId();
    }

    static std::string getSnapshot(
        const WorkItemMemento& memento
    ) {
        return memento.getDataSnapshot().getValue();
    }
};

// --------------------------------------------------
// TEST 1: Memento snapshot preservation
// --------------------------------------------------

TEST(WorkItemMementoTest, PreservesOriginalDataSnapshot) {

    // Step 1: Create original work item data.
    Data originalData(
        "Work item is waiting for approval."
    );

    EXPECT_EQ(
        originalData.getValue(),
        "Work item is waiting for approval."
    );

    // Step 2: Create a memento.
    WorkItemMemento memento(
        "ACT001",
        "WORK001",
        originalData,
        nullptr,
        nullptr
    );

    EXPECT_EQ(
        WorkItemMementoTestAccess::getActivityId(memento),
        "ACT001"
    );

    EXPECT_EQ(
        WorkItemMementoTestAccess::getWorkItemId(memento),
        "WORK001"
    );

    EXPECT_EQ(
        WorkItemMementoTestAccess::getSnapshot(memento),
        "Work item is waiting for approval."
    );

    // Step 3: Modify the original data.
    originalData.setValue(
        "Work item has been approved."
    );

    // Step 4: Verify that original data changed.
    EXPECT_EQ(
        originalData.getValue(),
        "Work item has been approved."
    );

    // Step 5: Verify that the snapshot remains unchanged.
    EXPECT_EQ(
        WorkItemMementoTestAccess::getSnapshot(memento),
        "Work item is waiting for approval."
    );
}

// --------------------------------------------------
// TEST 2: Identifier preservation
// --------------------------------------------------

TEST(WorkItemMementoTest, PreservesActivityAndWorkItemIdentifiers) {

    Data data("Pending approval.");

    WorkItemMemento memento(
        "ACT002",
        "WORK002",
        data,
        nullptr,
        nullptr
    );

    EXPECT_EQ(
        WorkItemMementoTestAccess::getActivityId(memento),
        "ACT002"
    );

    EXPECT_EQ(
        WorkItemMementoTestAccess::getWorkItemId(memento),
        "WORK002"
    );
}

// --------------------------------------------------
// TEST 3: Multiple updates do not affect snapshot
// --------------------------------------------------

TEST(WorkItemMementoTest, SnapshotRemainsUnchangedAfterMultipleUpdates) {

    Data originalData("Created");

    WorkItemMemento memento(
        "ACT003",
        "WORK003",
        originalData,
        nullptr,
        nullptr
    );

    // First update.
    originalData.setValue("Assigned");

    EXPECT_EQ(originalData.getValue(), "Assigned");

    EXPECT_EQ(
        WorkItemMementoTestAccess::getSnapshot(memento),
        "Created"
    );

    // Second update.
    originalData.setValue("In progress");

    EXPECT_EQ(originalData.getValue(), "In progress");

    EXPECT_EQ(
        WorkItemMementoTestAccess::getSnapshot(memento),
        "Created"
    );

    // Third update.
    originalData.setValue("Completed");

    EXPECT_EQ(originalData.getValue(), "Completed");

    EXPECT_EQ(
        WorkItemMementoTestAccess::getSnapshot(memento),
        "Created"
    );
}
