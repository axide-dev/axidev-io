cmake_minimum_required(VERSION 3.15)
project(axidev-io
    VERSION 0.3.0
    DESCRIPTION "Cross-platform layout-aware input/output library"
    LANGUAGES CXX C
)

option(AXIDEV_IO_BUILD_SHARED "Build axidev-io as a shared library" OFF)
option(AXIDEV_IO_BUILD_EXAMPLES "Build the example executables" OFF)
option(AXIDEV_IO_BUILD_TESTS "Build tests for axidev-io" OFF)
option(AXIDEV_IO_BUILD_TEST_CONSUMER "Build in-tree consumer demo" OFF)
option(AXIDEV_IO_EXPORT_COMPILE_COMMANDS "Enable compile_commands.json" ON)

if(AXIDEV_IO_BUILD_SHARED)
    set(BUILD_SHARED_LIBS ON)
endif()

if(AXIDEV_IO_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(AXIDEV_SOURCES
    src/keyboard/common/key_utils.cpp
    src/c_api.cpp
)

if(APPLE)
    list(APPEND AXIDEV_SOURCES
        src/keyboard/sender/sender_macos.mm
        src/keyboard/listener/listener_macos.mm
    )
elseif(WIN32)
    list(APPEND AXIDEV_SOURCES
        src/keyboard/sender/sender_windows.cpp
        src/keyboard/listener/listener_windows.cpp
    )
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBINPUT REQUIRED libinput)
    pkg_check_modules(LIBUDEV REQUIRED libudev)
    pkg_check_modules(XKBCOMMON REQUIRED xkbcommon)

    list(APPEND AXIDEV_SOURCES
        src/keyboard/sender/sender_uinput.cpp
        src/keyboard/listener/listener_linux.cpp
    )
endif()

if(AXIDEV_IO_BUILD_SHARED)
    add_library(axidev_io SHARED ${AXIDEV_SOURCES})
else()
    add_library(axidev_io STATIC ${AXIDEV_SOURCES})
endif()

target_include_directories(axidev_io
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(axidev_io PUBLIC cxx_std_20)

if(AXIDEV_IO_BUILD_SHARED)
    if(WIN32)
        target_compile_definitions(axidev_io PRIVATE axidev_io_EXPORTS)
    endif()
else()
    target_compile_definitions(axidev_io PUBLIC AXIDEV_IO_STATIC)
endif()

# Platform specific linkages
if(APPLE)
    target_link_libraries(axidev_io PUBLIC
        "-framework ApplicationServices"
        "-framework Carbon"
        "-framework Foundation"
        "-framework CoreGraphics"
    )
elseif(WIN32)
    target_link_libraries(axidev_io PUBLIC user32)
elseif(UNIX)
    target_include_directories(axidev_io PRIVATE
        ${LIBINPUT_INCLUDE_DIRS}
        ${XKBCOMMON_INCLUDE_DIRS}
        ${LIBUDEV_INCLUDE_DIRS}
    )
    target_link_libraries(axidev_io PRIVATE
        ${LIBINPUT_LIBRARIES}
        ${XKBCOMMON_LIBRARIES}
        ${LIBUDEV_LIBRARIES}
    )
endif()

find_package(Threads REQUIRED)
target_link_libraries(axidev_io PRIVATE Threads::Threads)

set_target_properties(axidev_io PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "include/axidev-io/core.hpp;include/axidev-io/keyboard/sender.hpp;include/axidev-io/keyboard/listener.hpp;include/axidev-io/c_api.h"
)

add_library(axidev::io ALIAS axidev_io)

# Installation
include(GNUInstallDirs)

install(TARGETS axidev_io
    EXPORT axidev-ioTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/axidev-io
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

install(EXPORT axidev-ioTargets
    FILE axidev-ioTargets.cmake
    NAMESPACE axidev::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/axidev-io
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/axidev-ioConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/axidev-ioConfig.cmake"
    "include(\"\${CMAKE_CURRENT_LIST_DIR}/axidev-ioTargets.cmake\")\n"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/axidev-ioConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/axidev-ioConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/axidev-io
)

export(EXPORT axidev-ioTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/axidev-ioTargets.cmake"
    NAMESPACE axidev::
)

# Examples
if(AXIDEV_IO_BUILD_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/example.cpp")
        add_executable(axidev-io-example examples/example.cpp)
        target_link_libraries(axidev-io-example PRIVATE axidev::io)
        install(TARGETS axidev-io-example RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/example_c.c")
        add_executable(axidev-io-example-c examples/example_c.c)
        set_source_files_properties(examples/example_c.c PROPERTIES LANGUAGE C)
        target_link_libraries(axidev-io-example-c PRIVATE axidev::io)
        install(TARGETS axidev-io-example-c RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
endif()

if(AXIDEV_IO_BUILD_TEST_CONSUMER)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_consumer/main.cpp")
        add_executable(axidev_io_consumer test_consumer/main.cpp)
        target_link_libraries(axidev_io_consumer PRIVATE axidev::io)
        target_compile_features(axidev_io_consumer PRIVATE cxx_std_20)
        install(TARGETS axidev_io_consumer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
endif()

if(AXIDEV_IO_BUILD_TESTS)
    enable_testing()
    if(TARGET axidev-io-example)
        add_test(NAME axidev-io-example-help COMMAND axidev-io-example --help)
    endif()
    if(TARGET axidev_io_consumer)
        add_test(NAME axidev-io-consumer-help COMMAND axidev_io_consumer --help)
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()

if(AXIDEV_IO_EXPORT_COMPILE_COMMANDS)
    add_custom_target(export-compile-commands
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PROJECT_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json
        COMMENT "Copy compile_commands.json to repository root"
    )
endif()

message(STATUS "axidev-io: Static build (AXIDEV_IO_BUILD_SHARED=${AXIDEV_IO_BUILD_SHARED})")
message(STATUS "axidev-io: Version ${PROJECT_VERSION}")