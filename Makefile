.PHONY: run clean rebuild build

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
