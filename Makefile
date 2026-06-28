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

# Clean rebuild and run (Editor)
run-editor: clean
	@cd build && cmake .. && make
	@cd bin && ./awesome-engine-editor

# Rebuild without cleaning and run
rebuild:
	@cd build && cmake .. && make
	@cd bin && ./awesome-engine

