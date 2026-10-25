# COS214-Prac6

Runtime decorators
- Added GetPriority to RuntimeComponent
- PriorityDecorator should naturally integrate with the iterator assuming the virtual GetChildren is called, its functionality is that it changes GetChildren method to arrange in the order of priorities they have
- Security decorator enforces security in the sense that if it is made needs the component to also have an attached attached participant, and requiredRoleId arguments in the constructor and it'll validate whether the attached arguments in the constructor allow for a setState call
Scenario 1: project approval
----------------------------

Build with `make Scenario1` (or `mingw32-make Scenario1` on this Windows setup), then run `./Scenario1` (PowerShell:
`.\Scenario1.exe`). The entry point is `scenario1.cpp`; `docs/main.cpp` is
an independent example and is not linked into this executable.

The scenario builds a workflow, creates runtime tasks with the factory, chooses
participants by role, rejects an unauthorized state change, and processes an
urgent technical review before a document review. Manager approval has the
highest priority but waits until both reviews finish. Commands drive the State
objects through assignment, starting and completion. The approval chain handles
the manager request, routing proposes the next task, a memento restores review
notes, and observers record the changes. Checks fail with exit code 1; a
successful run ends with `SCENARIO 1 PASSED`.

The main explicitly coordinates the unfinished integration points: it checks
security before commands (which take raw WorkItem pointers), updates display
state after checking each command's State object, and records command history.
WorkflowEngine::submit currently ignores the command result, so the scenario
checks the resulting State object itself. Engine startup/undo are placeholders
and are not demonstrated. Traversal uses getChildren(), not a new Iterator class.
The memento example restores Data; it does not claim to implement command undo.
