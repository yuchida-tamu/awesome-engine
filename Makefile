.PHONY: run clean rebuild build test test-verbose test-list

# Build only (no run)
build:
	@cd build && cmake .. && make

# Clean build artifacts
clean:
	@rm -rf bin

# Clean rebuild and run
run: clean
	@cd build && cmake .. && make
	@cd bin && ./awesome-engine

# Rebuild without cleaning and run
rebuild:
	@cd build && cmake .. && make
	@cd bin && ./awesome-engine

# Run tests
test: build
	@cd bin && ./run-tests

# Run tests with verbose output (show all assertions + timing)
test-verbose: build
	@cd bin && ./run-tests -s -d

# List all test cases
test-list: build
	@cd bin && ./run-tests --list-test-cases
