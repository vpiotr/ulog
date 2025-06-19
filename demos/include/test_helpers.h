#ifndef ULOG_TEST_HELPERS_H
#define ULOG_TEST_HELPERS_H

/**
 * @file test_helpers.h
 * @brief Reusable helper classes for ulog demos and tests
 * @author ulog team
 * @version 1.0.0
 * 
 * This file contains reusable helper classes that can be used across
 * multiple demos and tests to simplify common testing patterns.
 */

#include "ulog/ulog.h"
#include "../extensions/include/buffer_assertions.h"
#include <iostream>
#include <chrono>
#include <iomanip>

namespace ulog {
namespace demo {

/**
 * @brief RAII helper class for test setup and teardown
 * 
 * This class helps set up a logger for testing and automatically
 * cleans up after the test is complete. It provides a consistent
 * testing environment.
 */
class TestScope {
public:
    /**
     * @brief Constructor - sets up test environment
     * @param logger_name Name for the test logger
     * @param buffer_capacity Buffer capacity (0 = unlimited)
     * @param disable_console Whether to disable console output during test
     */
    explicit TestScope(const std::string& logger_name, 
                      size_t buffer_capacity = 100,
                      bool disable_console = true) 
        : logger_(ulog::getLogger(logger_name)), logger_name_(logger_name) {
        
        // Configure logger for testing
        logger_.enable_buffer(buffer_capacity);
        if (disable_console) {
            logger_.disable_console();
        }
        logger_.set_log_level(ulog::LogLevel::TRACE); // Capture all messages
        
        std::cout << "[TEST_SCOPE] Started test: " << logger_name_ << std::endl;
    }
    
    /**
     * @brief Destructor - cleans up test environment
     */
    ~TestScope() {
        // Clean up
        logger_.clear_buffer();
        logger_.disable_buffer();
        logger_.enable_console();
        logger_.set_log_level(ulog::LogLevel::INFO); // Reset to default
        
        std::cout << "[TEST_SCOPE] Finished test: " << logger_name_ << std::endl;
    }
    
    /**
     * @brief Get the logger reference
     * @return Reference to the test logger
     */
    ulog::Logger& logger() { return logger_; }
    
    /**
     * @brief Get buffer assertions helper
     * @return BufferAssertions instance for this logger
     */
    ulog::extensions::BufferAssertions assertions() {
        return ulog::extensions::BufferAssertions(logger_);
    }

private:
    ulog::Logger& logger_;
    std::string logger_name_;
};

/**
 * @brief Helper class for measuring performance of logging operations
 */
class PerformanceMeasurer {
public:
    /**
     * @brief Constructor
     * @param operation_name Name of the operation being measured
     */
    explicit PerformanceMeasurer(const std::string& operation_name)
        : operation_name_(operation_name), start_time_(std::chrono::high_resolution_clock::now()) {
        std::cout << "[PERF] Starting: " << operation_name_ << std::endl;
    }
    
    /**
     * @brief Destructor - prints elapsed time
     */
    ~PerformanceMeasurer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time_);
        
        std::cout << "[PERF] Completed: " << operation_name_ 
                  << " in " << duration.count() << " microseconds" << std::endl;
    }
    
    /**
     * @brief Get elapsed time since construction
     * @return Elapsed time in microseconds
     */
    long long elapsed_microseconds() const {
        auto current_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(
            current_time - start_time_).count();
    }

private:
    std::string operation_name_;
    std::chrono::high_resolution_clock::time_point start_time_;
};

/**
 * @brief Simple test runner for executing multiple test cases
 */
class SimpleTestRunner {
public:
    /**
     * @brief Constructor
     * @param suite_name Name of the test suite
     */
    explicit SimpleTestRunner(const std::string& suite_name) 
        : suite_name_(suite_name), passed_count_(0), failed_count_(0) {
        std::cout << "\n=== Test Suite: " << suite_name_ << " ===" << std::endl;
    }
    
    /**
     * @brief Destructor - prints test summary
     */
    ~SimpleTestRunner() {
        std::cout << "\n=== Test Suite Summary: " << suite_name_ << " ===" << std::endl;
        std::cout << "Passed: " << passed_count_ << std::endl;
        std::cout << "Failed: " << failed_count_ << std::endl;
        std::cout << "Total:  " << (passed_count_ + failed_count_) << std::endl;
        
        if (failed_count_ == 0) {
            std::cout << u8"All tests PASSED! ✓" << std::endl;
        } else {
            std::cout << u8"Some tests FAILED! ✗" << std::endl;
        }
    }
    
    /**
     * @brief Run a test case
     * @param test_name Name of the test
     * @param test_func Function containing the test logic
     */
    void run_test(const std::string& test_name, std::function<void()> test_func) {
        std::cout << "\n--- Running: " << test_name << " ---" << std::endl;
        
        try {
            test_func();
            std::cout << u8"✓ PASSED: " << test_name << std::endl;
            passed_count_++;
        } catch (const std::exception& e) {
            std::cout << u8"✗ FAILED: " << test_name << std::endl;
            std::cout << "  Error: " << e.what() << std::endl;
            failed_count_++;
        }
    }
    
    /**
     * @brief Get number of passed tests
     */
    size_t passed_count() const { return passed_count_; }
    
    /**
     * @brief Get number of failed tests
     */
    size_t failed_count() const { return failed_count_; }

private:
    std::string suite_name_;
    size_t passed_count_;
    size_t failed_count_;
};

/**
 * @brief Helper for generating test data
 */
class TestDataGenerator {
public:
    /**
     * @brief Generate log messages with different levels
     * @param logger Logger to write to
     * @param base_message Base message text
     * @param count Number of messages of each level to generate
     */
    static void generate_mixed_level_messages(ulog::Logger& logger, 
                                            const std::string& base_message,
                                            size_t count = 3) {
        for (size_t i = 0; i < count; ++i) {
            logger.trace(base_message + " TRACE " + std::to_string(i));
            logger.debug(base_message + " DEBUG " + std::to_string(i));
            logger.info(base_message + " INFO " + std::to_string(i));
            logger.warn(base_message + " WARN " + std::to_string(i));
            logger.error(base_message + " ERROR " + std::to_string(i));
        }
    }
    
    /**
     * @brief Generate messages with formatted parameters
     * @param logger Logger to write to
     * @param iterations Number of iterations
     */
    static void generate_formatted_messages(ulog::Logger& logger, size_t iterations = 5) {
        for (size_t i = 0; i < iterations; ++i) {
            logger.info("User {0} logged in at {1}", "user" + std::to_string(i), i * 1000);
            logger.warn("Warning: {0} attempts remaining", 5 - i);
            logger.error("Error code: {0}, details: {1}", 404 + i, "Not found");
        }
    }
    
    /**
     * @brief Generate a burst of messages quickly
     * @param logger Logger to write to  
     * @param count Number of messages to generate
     * @param message_prefix Prefix for each message
     */
    static void generate_burst_messages(ulog::Logger& logger, 
                                      size_t count, 
                                      const std::string& message_prefix = "Burst") {
        for (size_t i = 0; i < count; ++i) {
            logger.info("{0} message {1}", message_prefix, i);
        }
    }
};

} // namespace demo
} // namespace ulog

#endif // ULOG_TEST_HELPERS_H
