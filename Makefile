CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g

# Automatically find all .cpp files EXCEPT the ones with a main() function
CORE_SRCS = $(filter-out main.cpp scenario1.cpp BuilderComposite.cpp TestStateAndCommand.cpp test_%.cpp, $(wildcard *.cpp))

# Every test program (each test_*.cpp has its own main)
TESTS = $(basename $(wildcard test_*.cpp)) TestStateAndCommand

all: main scenario1 BuilderComposite $(TESTS)

# Build the full system demo (one project approval run through every pattern)
main: main.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build scenario 1 (project approval)
scenario1: scenario1.cpp $(CORE_SRCS)
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
	rm -f main scenario1 BuilderComposite $(TESTS)

# Phony targets (so "make clean" always works)
.PHONY: all clean run
