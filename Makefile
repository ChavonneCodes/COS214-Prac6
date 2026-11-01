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

clean:
	rm -f main scenario1 BuilderComposite $(TESTS) run_tests

# deep clean: executables + all .o files 
deepclean:
	rm -f *.o
	rm -f main scenario1 BuilderComposite $(TESTS) run_tests

# Phony targets (so "make clean" always works)
.PHONY: all clean run

# ── GoogleTest config ──────────────────────────────────────
GTEST_LIBS = -lgtest -lgtest_main -lpthread

# ── Test sources ───────────────────────────────────────────
TEST_SRCS = $(wildcard tests/*.cpp)

# ── Exclude files that have their own main() ──────────────
# These are the executables you also build — main.cpp, BuilderComposite.cpp,
# Scenario1.cpp, test_*.cpp, etc. We cannot link two mains.
EXCLUDE_SRCS = main.cpp scenario1.cpp BuilderComposite.cpp TestStateAndCommand.cpp $(wildcard test_*.cpp)

# ── App sources: everything in root EXCEPT files with main() ──
APP_SRCS = $(filter-out $(EXCLUDE_SRCS), $(wildcard *.cpp))
APP_OBJS = $(APP_SRCS:.cpp=.o)

# ── Build the test runner ─────────────────────────────────
run_tests: $(TEST_SRCS) $(APP_OBJS)
	$(CXX) $(CXXFLAGS) -I. $(TEST_SRCS) $(APP_OBJS) $(GTEST_LIBS) -o $@

# ── Convenience ───────────────────────────────────────────
tests: run_tests
	./run_tests