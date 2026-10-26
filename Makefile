CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# Automatically find all .cpp files EXCEPT the ones with a main() function
CORE_SRCS = $(filter-out main.cpp scenario1.cpp BuilderComposite.cpp TestStateAndCommand.cpp test_%.cpp, $(wildcard *.cpp))

# Every test program (each test_*.cpp has its own main)
TESTS = $(basename $(wildcard test_*.cpp)) TestStateAndCommand

all: main BuilderComposite $(TESTS)
# All non-main source files (no main() in any of these)
SRCS = ActivityDefinition.cpp ApprovalChainDefinition.cpp ApprovalStepDefinition.cpp \
       Approvalhandler.cpp AssignCommand.cpp AssignedState.cpp AuditObserver.cpp \
       AvailableState.cpp CancelCommand.cpp CancelledState.cpp CompleteCommand.cpp \
       CompletedState.cpp ConcreteWorkflowBuilder.cpp ConditionalRouting.cpp \
       CreatedState.cpp Data.cpp EscalateCommand.cpp EscalatedState.cpp EventBus.cpp \
       FailedState.cpp HistoryManager.cpp InMemoryDefinitionRepository.cpp InProgressState.cpp InstanceFactory.cpp \
       NotificationObserver.cpp Participant.cpp RejectCommand.cpp RejectedState.cpp \
       Request.cpp RoleApprovalHandler.cpp RoleBasedAssignment.cpp RoleDefintion.cpp \
       RuleBasedAssignment.cpp RuntimeComponent.cpp SequentialRouting.cpp StageDefinition.cpp StageInstance.cpp \
       StartCommand.cpp SubProcessDefinition.cpp SubProcessInstance.cpp \
       TimeoutEscalationHandler.cpp TransitionDefinition.cpp WorkFlowEvent.cpp \
       WorkItem.cpp WorkItemMemento.cpp WorkflowBuilder.cpp WorkflowDefinition.cpp \
       WorkflowDirector.cpp WorkflowEngine.cpp WorkflowInstance.cpp WorkloadBasedAssignment.cpp

OBJS = $(SRCS:.cpp=.o)

# Rebuild objects when included headers change.
$(OBJS): $(wildcard *.h)

# Build the full system demo (one project approval run through every pattern)
main: main.cpp $(CORE_SRCS)
# One target per test file (no duplicates!)
TESTS = Scenario1 BuilderComposite TestDecorator TestChainOfResponsibility TestRouting TestObserver TestStrategy TestMemento

.PHONY: all clean

all: $(TESTS)

.PHONY: Scenario1 scenario1
Scenario1 scenario1: scenario1.exe

scenario1.exe: scenario1.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

BuilderComposite: BuilderComposite.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build the Builder Composite application
BuilderComposite: BuilderComposite.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build any test program from its own source file
test_%: test_%.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

TestStateAndCommand: TestStateAndCommand.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build and run the demo
run: main
	./main

# Clean up the executables
clean:
	rm -f main BuilderComposite $(TESTS) scenario1.exe

# Phony targets (so "make clean" always works)
.PHONY: all clean run

	rm -f $(OBJS) $(TESTS)
clean-t:
	del /Q *.o *.exe
