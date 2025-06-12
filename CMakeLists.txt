cmake_minimum_required(VERSION 3.16)
project(ulog VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

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

add_executable(test_buffer
    tests/test_buffer.cpp
)
target_link_libraries(test_buffer ulog)

add_executable(test_formatter
    tests/test_formatter.cpp
)
target_link_libraries(test_formatter ulog)

add_executable(test_observer
    tests/test_observer.cpp
)
target_link_libraries(test_observer ulog)

# Demo executables
add_executable(ulog_demo
    demos/demo_main.cpp
)
target_link_libraries(ulog_demo ulog)

add_executable(demo_file_observer
    demos/demo_file_observer.cpp
)
target_link_libraries(demo_file_observer ulog)

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
