cmake_minimum_required(VERSION 3.16)
project(ulog VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(MSVC)
    # Enable UTF-8 source and execution character sets for Visual Studio
    add_compile_options(/utf-8)
    add_compile_options(/W4)
    # Ensure proper UTF-8 handling in runtime
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
elseif(WIN32)
    # For other Windows compilers (MinGW, Clang, etc.)
    add_compile_options(-Wall -Wextra -Wpedantic)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Function to configure UTF-8 settings for all targets
function(configure_utf8_target target_name)
    if(MSVC)
        # Set UTF-8 for source files and execution character set
        target_compile_options(${target_name} PRIVATE /utf-8)
        # Add UTF-8 BOM handling for Visual Studio
        target_compile_definitions(${target_name} PRIVATE UNICODE _UNICODE)
    endif()
endfunction()

# Set build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Header-only library target
add_library(ulog INTERFACE)
target_include_directories(ulog INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(ulog INTERFACE cxx_std_17)

# Enable testing
enable_testing()

# Find threads (required for std::mutex)
find_package(Threads REQUIRED)
target_link_libraries(ulog INTERFACE Threads::Threads)

# Test executables - each module independent
add_executable(test_logger
    tests/test_logger.cpp
)
target_link_libraries(test_logger ulog)
configure_utf8_target(test_logger)

add_executable(test_buffer
    tests/test_buffer.cpp
)
target_link_libraries(test_buffer ulog)
configure_utf8_target(test_buffer)

add_executable(test_formatter
    tests/test_formatter.cpp
)
target_link_libraries(test_formatter ulog)
configure_utf8_target(test_formatter)

add_executable(test_observer
    tests/test_observer.cpp
)
target_link_libraries(test_observer ulog)
configure_utf8_target(test_observer)

# Demo executables
add_executable(ulog_demo
    demos/demo_main.cpp
)
target_link_libraries(ulog_demo ulog)
configure_utf8_target(ulog_demo)

add_executable(demo_file_observer
    demos/demo_file_observer.cpp
)
target_link_libraries(demo_file_observer ulog)
configure_utf8_target(demo_file_observer)

add_executable(demo_log_level_filtering
    demos/demo_log_level_filtering.cpp
)
target_link_libraries(demo_log_level_filtering ulog)
configure_utf8_target(demo_log_level_filtering)

add_executable(demo_custom_formatting
    demos/demo_custom_formatting.cpp
)
target_link_libraries(demo_custom_formatting ulog)
configure_utf8_target(demo_custom_formatting)

add_executable(demo_auto_flushing
    demos/demo_auto_flushing.cpp
)
target_link_libraries(demo_auto_flushing ulog)
configure_utf8_target(demo_auto_flushing)

add_executable(demo_debug_scope
    demos/demo_debug_scope.cpp
)
target_link_libraries(demo_debug_scope ulog)
configure_utf8_target(demo_debug_scope)

add_executable(demo_slow_op_guard
    demos/demo_slow_op_guard.cpp
)
target_link_libraries(demo_slow_op_guard ulog)
target_include_directories(demo_slow_op_guard PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/demos/extensions/include
)
configure_utf8_target(demo_slow_op_guard)

add_executable(demo_cerr_observer
    demos/demo_cerr_observer.cpp
)
target_link_libraries(demo_cerr_observer ulog)
configure_utf8_target(demo_cerr_observer)

add_executable(demo_exception_formatting
    demos/demo_exception_formatting.cpp
)
target_link_libraries(demo_exception_formatting ulog)
configure_utf8_target(demo_exception_formatting)

add_executable(demo_container_formatting
    demos/demo_container_formatting.cpp
)
target_link_libraries(demo_container_formatting ulog)
configure_utf8_target(demo_container_formatting)

add_executable(demo_ustr_integration
    demos/demo_ustr_integration.cpp
)
target_link_libraries(demo_ustr_integration ulog)
# Add the demos directory to the include path for demo_ustr_integration
# so it can find "ustr/ustr.h"
target_include_directories(demo_ustr_integration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/demos)
configure_utf8_target(demo_ustr_integration)

add_executable(demo_buffer_assertions
    demos/demo_buffer_assertions.cpp
)
target_link_libraries(demo_buffer_assertions ulog)
# Add the demos directories to the include path for demo_buffer_assertions
# so it can find "buffer_assertions.h" and "test_helpers.h"
target_include_directories(demo_buffer_assertions PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/demos/include
    ${CMAKE_CURRENT_SOURCE_DIR}/demos/extensions/include
)
configure_utf8_target(demo_buffer_assertions)

add_executable(demo_buffer_stats
    demos/demo_buffer_stats.cpp
)
target_link_libraries(demo_buffer_stats ulog)
# Add the demos directories to the include path for demo_buffer_stats
# so it can find "buffer_stats.h", "buffer_stats_reporter.h" and "test_helpers.h"
target_include_directories(demo_buffer_stats PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/demos/include
    ${CMAKE_CURRENT_SOURCE_DIR}/demos/extensions/include
)
configure_utf8_target(demo_buffer_stats)

add_executable(demo_threaded_buffer_stats
    demos/demo_threaded_buffer_stats.cpp
)
target_link_libraries(demo_threaded_buffer_stats ulog)
# Add the demos directories to the include path for demo_threaded_buffer_stats
# so it can find extension headers and helpers
target_include_directories(demo_threaded_buffer_stats PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/demos/include
    ${CMAKE_CURRENT_SOURCE_DIR}/demos/extensions/include
)
configure_utf8_target(demo_threaded_buffer_stats)

add_executable(demo_buffer_benchmark_no_mutex
    benchmarks/demo_buffer_benchmark_no_mutex.cpp
)
target_link_libraries(demo_buffer_benchmark_no_mutex ulog)
target_include_directories(demo_buffer_benchmark_no_mutex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)
configure_utf8_target(demo_buffer_benchmark_no_mutex)

add_executable(demo_buffer_benchmark_with_mutex
    benchmarks/demo_buffer_benchmark_with_mutex.cpp
)
target_link_libraries(demo_buffer_benchmark_with_mutex ulog)
target_include_directories(demo_buffer_benchmark_with_mutex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)
configure_utf8_target(demo_buffer_benchmark_with_mutex)

add_executable(demo_observer_benchmark_with_mutex
    benchmarks/demo_observer_benchmark_with_mutex.cpp
)
target_link_libraries(demo_observer_benchmark_with_mutex ulog)
target_include_directories(demo_observer_benchmark_with_mutex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)
configure_utf8_target(demo_observer_benchmark_with_mutex)

add_executable(demo_observer_benchmark_no_mutex
    benchmarks/demo_observer_benchmark_no_mutex.cpp
)
target_link_libraries(demo_observer_benchmark_no_mutex ulog)
target_include_directories(demo_observer_benchmark_no_mutex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)
configure_utf8_target(demo_observer_benchmark_no_mutex)

add_executable(demo_message_supplier
    demos/demo_message_supplier.cpp
)
target_link_libraries(demo_message_supplier ulog)
configure_utf8_target(demo_message_supplier)

add_executable(demo_windows_utf8
    demos/demo_windows_utf8.cpp
)
target_link_libraries(demo_windows_utf8 ulog)
configure_utf8_target(demo_windows_utf8)


# Add tests to CTest
add_test(NAME test_logger COMMAND test_logger)
add_test(NAME test_buffer COMMAND test_buffer)
add_test(NAME test_formatter COMMAND test_formatter)
add_test(NAME test_observer COMMAND test_observer)

# Install configuration
install(DIRECTORY include/ DESTINATION include)
install(TARGETS ulog EXPORT ulogTargets)
install(EXPORT ulogTargets
    FILE ulogTargets.cmake
    NAMESPACE ulog::
    DESTINATION lib/cmake/ulog
)

# Create config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ulogConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ulogConfig.cmake"
    INSTALL_DESTINATION lib/cmake/ulog
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ulogConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ulogConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ulogConfigVersion.cmake"
    DESTINATION lib/cmake/ulog
)
