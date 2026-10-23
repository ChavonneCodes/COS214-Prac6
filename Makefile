CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g

# All non-main source files (no main() in any of these)
SRCS = ActivityDefinition.cpp ApprovalChainDefinition.cpp ApprovalStepDefinition.cpp \
       Approvalhandler.cpp AssignCommand.cpp AssignedState.cpp AuditObserver.cpp \
       AvailableState.cpp CancelCommand.cpp CancelledState.cpp CompleteCommand.cpp \
       CompletedState.cpp ConcreteWorkflowBuilder.cpp ConditionalRouting.cpp \
       CreatedState.cpp Data.cpp EscalateCommand.cpp EscalatedState.cpp EventBus.cpp \
       FailedState.cpp HistoryManager.cpp InProgressState.cpp InstanceFactory.cpp \
       NotificationObserver.cpp Participant.cpp RejectCommand.cpp RejectedState.cpp \
       Request.cpp RoleApprovalHandler.cpp RoleBasedAssignment.cpp RoleDefintion.cpp \
       RuleBasedAssignment.cpp SequentialRouting.cpp StageDefinition.cpp StageInstance.cpp \
       StartCommand.cpp SubProcessDefinition.cpp SubProcessInstance.cpp \
       TimeoutEscalationHandler.cpp TransitionDefinition.cpp WorkFlowEvent.cpp \
       WorkItem.cpp WorkItemMemento.cpp WorkflowBuilder.cpp WorkflowDefinition.cpp \
       WorkflowDirector.cpp WorkflowEngine.cpp WorkloadBasedAssignment.cpp \
       RuntimeComponentIterator.cpp

OBJS = $(SRCS:.cpp=.o)

# One target per test file (no duplicates!)
TESTS = BuilderComposite TestDecorator TestChainOfResponsibility TestRouting TestObserver TestStrategy TestMemento TestIterator

.PHONY: all clean

all: $(TESTS)

BuilderComposite: BuilderComposite.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

TestDecorator: test_decorator.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

TestChainOfResponsibility: test_ChainOfResponsibility.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

TestRouting: test_routing.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

TestObserver: test_observer.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

TestStrategy: test_strategy.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

TestMemento: test_memento.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

TestIterator: test_iterator.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TESTS)
