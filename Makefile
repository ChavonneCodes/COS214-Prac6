# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# Automatically find all .cpp files EXCEPT the ones with a main() function
CORE_SRCS = $(filter-out test_routing.cpp BuilderComposite.cpp test_observer.cpp test_strategy.cpp, $(wildcard *.cpp))

# The three executable targets
all: BuilderComposite test_observer test_strategy test_routing

# Build the Builder Compositeapplication
BuilderComposite: BuilderComposite.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build the Observer test
test_observer: test_observer.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build the Strategy test
test_strategy: test_strategy.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

test_routing: test_routing.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Clean up the executables
clean:
	rm -f BuilderComposite test_observer test_strategy test_routing

# Phony targets (so "make clean" always works)
.PHONY: all clean