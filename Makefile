# Makefile - convenience tasks for local development
#
# Usage examples:
#   make configure                           # configure in Debug mode (exports compile_commands.json)
#   make configure CMAKE_BUILD_TYPE=Release
#   make build                               # build (after configure)
#   make test                                # build and run tests
#   make integration-test                    # run integration tests (interactive)
#   make run-unit-tests                      # run unit tests binary directly
#   make export-compile-commands             # copy compile_commands.json to repo root
#   make clean

CMAKE ?= cmake
CTEST ?= ctest

BUILD_DIR ?= build
CMAKE_BUILD_TYPE ?= Debug
CMAKE_ARGS ?=

# Number of parallel build jobs (try a few common probes)
JOBS ?= $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

ifeq ($(OS),Windows_NT)
EXE_EXT := .exe
else
EXE_EXT :=
endif

RUN_UNIT_TESTS := $(BUILD_DIR)/tests/axidev-io-unit-tests$(EXE_EXT)

.PHONY: all configure configure-release build test integration-test run-unit-tests export-compile-commands clean help

all: build

configure:
	@echo "Configuring (Build dir = $(BUILD_DIR), Type = $(CMAKE_BUILD_TYPE))..."
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $(CMAKE_ARGS)

configure-release:
	$(MAKE) configure CMAKE_BUILD_TYPE=Release

build:
	@echo "Building (parallel jobs = $(JOBS))..."
	$(CMAKE) --build $(BUILD_DIR) --parallel $(JOBS)

test:
	@echo "Configuring with tests enabled..."
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DAXIDEV_IO_BUILD_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $(CMAKE_ARGS)
	@echo "Building (parallel jobs = $(JOBS))..."
	$(CMAKE) --build $(BUILD_DIR) --parallel $(JOBS)
	@echo "Running tests..."
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

integration-test:
	@echo "Running integration tests (interactive)..."
	$(CMAKE) -S . -B $(BUILD_DIR) -DAXIDEV_IO_BUILD_TESTS=ON -DAXIDEV_IO_BUILD_INTEGRATION_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
	$(CMAKE) --build $(BUILD_DIR) --parallel $(JOBS)
	AXIDEV_IO_RUN_INTEGRATION_TESTS=1 AXIDEV_IO_INTERACTIVE=1 $(CTEST) --test-dir $(BUILD_DIR) -R axidev-io-integration-tests -C Debug --output-on-failure -V

run-unit-tests:
	@if [ ! -x "$(RUN_UNIT_TESTS)" ]; then \
		echo "Unit tests binary not found. Configuring and building with tests enabled..."; \
		$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DAXIDEV_IO_BUILD_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $(CMAKE_ARGS); \
		$(CMAKE) --build $(BUILD_DIR) --parallel $(JOBS); \
	fi
	@echo "Running unit tests binary..."
	@if [ -x "$(RUN_UNIT_TESTS)" ]; then \
		"$(RUN_UNIT_TESTS)"; \
	else \
		echo "Unit tests binary not found: $(RUN_UNIT_TESTS)"; \
		exit 1; \
	fi

export-compile-commands:
	@echo "Copying compile_commands.json from $(BUILD_DIR) to repository root (if present)..."
	$(CMAKE) -E copy_if_different $(BUILD_DIR)/compile_commands.json compile_commands.json
	@echo "Done."

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) compile_commands.json

help:
	@echo ""
	@echo "Common targets:"
	@echo "  configure                 Configure the project (Debug by default)."
	@echo "                           Pass CMAKE_ARGS to forward -D flags to CMake."
	@echo "  configure-release         Configure with Release build type."
	@echo "  build                     Build the project."
	@echo "  test                      Build and run tests (ctest)."
	@echo "  integration-test          Configure, build, and run integration tests (interactive)."
	@echo "  run-unit-tests            Run unit tests binary directly (must build first)."
	@echo "  run-consumer              Run the in-tree consumer (use RUN_ARGS to pass args)."
	@echo "  export-compile-commands   Copy build/compile_commands.json to repository root."
	@echo "  clean                     Remove build dir and compile_commands.json."
	@echo ""
