
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "Data.h"
#include "HistoryManager.h"
#include "WorkItemMemento.h"

using namespace std;

int main() {
    cout << "========== UNIT TESTS ==========\n\n";

    // ==========================================
    // TEST 1: Data default constructor
    // ==========================================
    cout << "Test 1: Data default constructor\n";

    Data data1;
    assert(data1.getValue() == "");

    cout << "PASS: Default value is an empty string.\n\n";


    // ==========================================
    // TEST 2: Data parameterised constructor
    // ==========================================
    cout << "Test 2: Data parameterised constructor\n";

    Data data2("Workflow information");
    assert(data2.getValue() == "Workflow information");

    cout << "PASS: Constructor stores the supplied value.\n\n";


    // ==========================================
    // TEST 3: Data setter
    // ==========================================
    cout << "Test 3: Data setter\n";

    data2.setValue("Updated workflow information");
    assert(data2.getValue() == "Updated workflow information");

    cout << "PASS: setValue updates the stored value.\n\n";


    // ==========================================
    // TEST 4: HistoryManager initially empty
    // ==========================================
    cout << "Test 4: HistoryManager initially empty\n";

    HistoryManager history;

    assert(history.getAll().empty());

    cout << "PASS: History starts empty.\n\n";


    // ==========================================
    // TEST 5: Pop from empty history
    // ==========================================
    cout << "Test 5: Pop from empty history\n";

    WorkflowAction* action = history.pop();

    assert(action == nullptr);

    cout << "PASS: Popping empty history returns nullptr.\n\n";


    // ==========================================
    // TEST 6: Push and pop a history entry
    // ==========================================
    cout << "Test 6: Push and pop a history entry\n";

    // nullptr is used here because the interface and
    // constructors of WorkflowAction were not provided.
    history.push(nullptr);

    assert(history.getAll().size() == 1);

    action = history.pop();

    assert(action == nullptr);
    assert(history.getAll().empty());

    cout << "PASS: Push adds an entry and pop removes it.\n\n";


    // ==========================================
    // TEST 7: WorkItemMemento snapshot
    // ==========================================
    cout << "Test 7: WorkItemMemento snapshot\n";

    Data originalData("Original work-item data");

    WorkItemMemento memento(
        "ACT-001",
        "WI-001",
        originalData,
        nullptr
    );

    assert(memento.getActivityId() == "ACT-001");
    assert(memento.getWorkItemId() == "WI-001");
    assert(memento.getDataSnapshot().getValue()
           == "Original work-item data");
    assert(memento.getStateKind() == nullptr);

    cout << "PASS: Memento stores the IDs, data snapshot, "
         << "and state pointer.\n\n";


    // ==========================================
    // TEST 8: Snapshot remains independent
    // ==========================================
    cout << "Test 8: Snapshot remains independent\n";

    originalData.setValue("Modified work-item data");

    assert(memento.getDataSnapshot().getValue()
           == "Original work-item data");

    cout << "PASS: Changing the original Data object "
         << "does not change the snapshot.\n\n";


    cout << "========== ALL TESTS PASSED ==========\n";

    return 0;
}
