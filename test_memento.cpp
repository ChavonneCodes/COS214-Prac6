
/**
 * @file test_history_manager.cpp
 * @brief Tests the WorkItemMemento and HistoryManager classes.
 *
 * Compile this file with the implementations of HistoryManager,
 * WorkItemMemento, Data, WorkItemState, and CreatedState.
 */

#include <cassert>
#include <iostream>

#include "HistoryManager.h"
#include "WorkItemMemento.h"
#include "CreatedState.h"
#include "Data.h"

using namespace std;

/**
 * @brief Tests saving and retrieving snapshots.
 */
void testSaveAndGetLast() {
    cout << "Test: save() and getLast()" << endl;

    HistoryManager history;

    Data data("Initial data");
    CreatedState state;

    WorkItemMemento first(
        "ACT01",
        "WI01",
        data,
        &state
    );

    history.save(first);

    assert(history.getAll().size() == 1);

    WorkItemMemento last = history.getLast();

    // Confirm the saved snapshot can be copied and retrieved.
    assert(history.getAll().size() == 1);

    cout << "PASS: Snapshot saved and retrieved." << endl;
}

/**
 * @brief Tests that pop() removes the latest snapshot.
 */
void testPop() {
    cout << "Test: pop()" << endl;

    HistoryManager history;

    Data data1("First snapshot");
    Data data2("Second snapshot");

    CreatedState state;

    WorkItemMemento first(
        "ACT01",
        "WI01",
        data1,
        &state
    );

    WorkItemMemento second(
        "ACT01",
        "WI01",
        data2,
        &state
    );

    history.save(first);
    history.save(second);

    assert(history.getAll().size() == 2);

    WorkItemMemento removed = history.pop();

    assert(history.getAll().size() == 1);

    // The first snapshot should now be the most recent.
    WorkItemMemento remaining = history.getLast();

    assert(history.getAll().size() == 1);

    cout << "PASS: pop() removes the latest snapshot." << endl;
}

/**
 * @brief Tests behaviour when history is empty.
 */
void testEmptyHistory() {
    cout << "Test: empty history" << endl;

    HistoryManager history;

    assert(history.getAll().empty());

    // Both operations should safely return default mementos.
    WorkItemMemento last = history.getLast();
    WorkItemMemento removed = history.pop();

    assert(history.getAll().empty());

    cout << "PASS: Empty history handled safely." << endl;
}

/**
 * @brief Tests copying and destroying mementos.
 *
 * The test is especially useful for detecting double deletion
 * when compiled with AddressSanitizer.
 */
void testMementoCopying() {
    cout << "Test: memento copying" << endl;

    Data data("Copy test");
    CreatedState state;

    WorkItemMemento original(
        "ACT01",
        "WI01",
        data,
        &state
    );

    {
        WorkItemMemento copy(original);
        WorkItemMemento assigned;

        assigned = copy;

        // All three mementos should own independent state clones.
    }

    // Original must remain valid after the copies are destroyed.
    WorkItemMemento anotherCopy(original);

    cout << "PASS: Mementos copied and destroyed." << endl;
}

/**
 * @brief Runs all tests.
 *
 * @return 0 if every assertion passes.
 */
int main() {
    testSaveAndGetLast();
    testPop();
    testEmptyHistory();
    testMementoCopying();

    cout << "\nAll tests passed." << endl;

    return 0;
}
