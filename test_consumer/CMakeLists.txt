cmake_minimum_required(VERSION 3.15)
project(axidev-io-consumer LANGUAGES CXX)

# Simple consumer CMake file for demonstrating how to consume an installed
# axidev-io package via find_package(axidev-io CONFIG REQUIRED).
#
# Usage examples:
#   # If axidev-io has been installed system-wide (e.g. /usr/local), a simple:
#   cmake -S . -B build
#
#   # If axidev-io is installed to a non-standard prefix (for example the local
#   # build/install tree from this repository), point CMake to it:
#   cmake -S . -B build -DAXIDEV_IO_ROOT=/path/to/axidev-io/build/install
#
#   # Alternatively use CMAKE_PREFIX_PATH directly:
#   cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/axidev-io/build/install
#
# After configure:
#   cmake --build build
#
# Then run the produced executable:
#   ./build/axidev_io_consumer
#
# This CMakeLists assumes there is a simple `main.cpp` in this directory that
# uses the public axidev-io headers (include the specific headers you need, e.g.):
#   #include <axidev-io/sender.hpp>
#   #include <axidev-io/listener.hpp>

option(AXIDEV_IO_ROOT "Root prefix where axidev-io is installed (sets CMAKE_PREFIX_PATH)" "")

if (AXIDEV_IO_ROOT)
    list(APPEND CMAKE_PREFIX_PATH "${AXIDEV_IO_ROOT}")
    message(STATUS "Appended AXIDEV_IO_ROOT to CMAKE_PREFIX_PATH: ${AXIDEV_IO_ROOT}")
endif()

# Attempt to find an installed axidev-io using the exported CMake config files.
# This requires axidev-io to have been installed (e.g. via `cmake --install`).
find_package(axidev-io CONFIG REQUIRED)

# Simple executable that links to the public target `axidev::io`.
add_executable(axidev_io_consumer main.cpp)

# Link against the imported target exposed by axidev-io. This will transitively
# bring include directories and any link requirements into this target.
target_link_libraries(axidev_io_consumer PRIVATE axidev::io)

# Ensure C++20
target_compile_features(axidev_io_consumer PRIVATE cxx_std_20)

message(STATUS "axidev-io consumer: linked to axidev::io")
