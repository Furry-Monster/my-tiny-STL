# Makefile for Monster STL Library Tests

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -g
INCLUDES = -I.

# Source files
HEADERS = $(wildcard *.hpp)
TEST_SOURCES = $(wildcard *_test.cpp)
TEST_TARGETS = $(TEST_SOURCES:.cpp=)

# Default target
all: $(TEST_TARGETS)

# Rule to build test executables
%_test: %_test.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

# Clean target
clean:
	rm -f $(TEST_TARGETS)

# Run all tests
test: $(TEST_TARGETS)
	@echo "Running all tests..."
	@for test in $(TEST_TARGETS); do \
		echo "Running $$test..."; \
		./$$test; \
		echo ""; \
	done

# Individual test targets
function_test: function_test.cpp function.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

raii_test: raii_test.cpp raii.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

array_test: array_test.cpp array.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

vector_test: vector_test.cpp vector.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

list_test: list_test.cpp list.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

# Debug builds
debug: CXXFLAGS += -DDEBUG -O0
debug: $(TEST_TARGETS)

# Help target
help:
	@echo "Available targets:"
	@echo "  all         - Build all test executables"
	@echo "  test        - Build and run all tests"
	@echo "  clean       - Remove all built executables"
	@echo "  debug       - Build with debug flags"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Individual test targets:"
	@echo "  function_test - Build function library test"
	@echo "  raii_test     - Build RAII library test"
	@echo "  array_test    - Build array library test"
	@echo "  vector_test   - Build vector library test"
	@echo "  list_test     - Build list library test"

.PHONY: all clean test debug help