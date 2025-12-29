# CMake configuration for unit tests (Catch2)
#
# This file is intended to be included via `add_subdirectory(tests)` from the
# top-level CMake when AXIDEV_IO_BUILD_TESTS=ON.
#
# It fetches Catch2 (v3) via FetchContent and builds a small test executable
# that links against the `axidev::io` target produced by the main project.

cmake_minimum_required(VERSION 3.15)

# If tests are disabled at configure time, do nothing here. The top-level
# project typically checks this, but being defensive makes the subdir usable
# standalone too.
if(NOT AXIDEV_IO_BUILD_TESTS)
    message(STATUS "AXIDEV_IO_BUILD_TESTS is OFF; skipping unit tests subdirectory")
    return()
endif()

include(FetchContent)

# Prefer an already-available Catch2; otherwise fetch a known release.
if(NOT TARGET Catch2::Catch2WithMain)
    FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )
    # This makes the Catch2 targets available (Catch2::Catch2WithMain)
    FetchContent_MakeAvailable(catch2)
endif()

# Optional: integration tests are interactive / long-running and should be
# separated from the regular unit test binary. Enable them with:
#   -DAXIDEV_IO_BUILD_INTEGRATION_TESTS=ON
option(AXIDEV_IO_BUILD_INTEGRATION_TESTS "Build integration tests (interactive / long-running)" OFF)

# Unit test executable (only lightweight, non-interactive tests)
add_executable(axidev-io-unit-tests
    test_key_utils.cpp
    test_c_api.cpp
)

target_link_libraries(axidev-io-unit-tests
    PRIVATE
        axidev::io
        Catch2::Catch2WithMain
)

# Ensure modern C++ standard (project uses C++20)
target_compile_features(axidev-io-unit-tests PRIVATE cxx_std_20)

# Register with CTest
enable_testing()
add_test(NAME axidev-io-unit-tests COMMAND axidev-io-unit-tests)
# Label the unit test so runners can select it explicitly if desired
set_property(TEST axidev-io-unit-tests PROPERTY LABELS unit)

# Optional integration test executable (built when AXIDEV_IO_BUILD_INTEGRATION_TESTS=ON)
if(AXIDEV_IO_BUILD_INTEGRATION_TESTS)
    add_executable(axidev-io-integration-tests
        test_integration_sender.cpp
        test_integration_listener.cpp
    )

    target_link_libraries(axidev-io-integration-tests
        PRIVATE
            axidev::io
            Catch2::Catch2WithMain
    )

    # Ensure consistent language standard
    target_compile_features(axidev-io-integration-tests PRIVATE cxx_std_20)

    add_test(NAME axidev-io-integration-tests COMMAND axidev-io-integration-tests)
    # Mark the test with a label so runners can select it explicitly if desired
    set_property(TEST axidev-io-integration-tests PROPERTY LABELS integration)
endif()
