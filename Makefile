# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# Automatically find all .cpp files EXCEPT the ones with a main() function
CORE_SRCS = $(filter-out BuilderComposite.cpp test_observer.cpp test_strategy.cpp TestStateAndCommand.cpp, $(wildcard *.cpp))

# The three executable targets
all: BuilderComposite test_observer test_strategy TestStateAndCommand

# Build the Builder Compositeapplication
BuilderComposite: BuilderComposite.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build the Observer test
test_observer: test_observer.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build the Strategy test
test_strategy: test_strategy.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

TestStateAndCommand: TestStateAndCommand.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Clean up the executables
clean:
	rm -f BuilderComposite test_observer test_strategy TestStateAndCommand

# Phony targets (so "make clean" always works)
.PHONY: all clean
