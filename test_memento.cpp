
#include <iostream>
#include <cassert>
#include "WorkItemMemento.h"
#include "Data.h"

using namespace std;

/**
 * @class WorkItemMementoTest
 * @brief Tests the behaviour of the WorkItemMemento class.
 *
 * This test class is a friend of WorkItemMemento so that it can
 * access the private getters without making them publicly accessible.
 */
class WorkItemMementoTest {
private:

    /**
     * @brief Displays a test result.
     */
    static void pass(const string& message) {
        cout << "[PASS] " << message << "\n\n";
    }

public:

    /**
     * @brief Demonstrates that a memento stores a snapshot of data.
     *
     * The original data is changed after the memento is created.
     * The snapshot should retain the original value.
     */
    static void testSnapshotPreservation() {

        cout << "========================================\n";
        cout << "TEST: MEMENTO SNAPSHOT PRESERVATION\n";
        cout << "========================================\n\n";

        // Step 1: Create the original work item data.
        Data originalData("Work item is waiting for approval.");

        cout << "STEP 1: Creating original work item data\n";
        cout << "Original data: "
             << originalData.getValue() << "\n\n";

        // Step 2: Create a memento to save the current state.
        WorkItemMemento memento(
            "ACT001",
            "WORK001",
            originalData,
            nullptr
        );

        cout << "STEP 2: Creating a memento\n";
        cout << "Activity ID: " << memento.getActivityId() << "\n";
        cout << "Work Item ID: " << memento.getWorkItemId() << "\n";
        cout << "Saved snapshot: "
             << memento.getDataSnapshot().getValue() << "\n\n";

        // Step 3: Change the original work item data.
        cout << "STEP 3: Updating the original work item data\n";

        originalData.setValue("Work item has been approved.");

        cout << "Updated original data: "
             << originalData.getValue() << "\n\n";

        // Step 4: Display both values to demonstrate preservation.
        cout << "STEP 4: Comparing original data with saved snapshot\n";

        cout << "Original data: "
             << originalData.getValue() << "\n";

        cout << "Saved snapshot: "
             << memento.getDataSnapshot().getValue() << "\n\n";

        // Step 5: Verify that the snapshot was not changed.
        assert(
            memento.getDataSnapshot().getValue()
            == "Work item is waiting for approval."
        );

        assert(
            originalData.getValue()
            == "Work item has been approved."
        );

        pass("Memento preserved the original data after modification.");
    }

    /**
     * @brief Runs all WorkItemMemento tests.
     */
    static void runAllTests() {

        testSnapshotPreservation();

        cout << "========================================\n";
        cout << "ALL MEMENTO TESTS COMPLETED\n";
        cout << "========================================\n";
    }
};

/**
 * @brief Main entry point for the memento tests.
 */
int main() {

    WorkItemMementoTest::runAllTests();

    return 0;
}
