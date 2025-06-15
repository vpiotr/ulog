/**
 * @file demo_buffer_assertions.cpp
 * @brief Demonstration of BufferAssertions extension for testing log buffer contents
 * @author ulog team
 * @version 1.0.0
 * 
 * This demo shows how to use the BufferAssertions extension to validate
 * log buffer contents in tests. It demonstrates various assertion methods
 * and testing patterns that can be reused in your own projects.
 * 
 * Key features demonstrated:
 * - Basic buffer assertions (size, empty, content)
 * - Log level filtering and counting
 * - Message pattern matching with regex
 * - Custom predicates for complex validations
 * - Performance testing helpers
 * - Comprehensive test scenarios
 * 
 * Compilation:
 * g++ -std=c++17 -I../include -I../demos/include -I../demos/extensions/include \
 *     demo_buffer_assertions.cpp -o demo_buffer_assertions -pthread
 */

#include "ulog/ulog.h"
#include "buffer_assertions.h"
#include "test_helpers.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

using namespace ulog;
using namespace ulog::extensions;
using namespace ulog::demo;

// Forward declarations for test functions
void demo_basic_assertions();
void demo_message_content_assertions();
void demo_log_level_assertions();
void demo_pattern_matching();
void demo_custom_predicates();
void demo_chronological_ordering();
void demo_performance_testing();
void demo_error_handling();
void demo_comprehensive_scenario();

int main() {
    std::cout << "=== BufferAssertions Demo ===" << std::endl;
    std::cout << "This demo shows how to use BufferAssertions for testing log buffer contents." << std::endl;
    std::cout << std::endl;
    
    // Run all demonstrations
    SimpleTestRunner runner("BufferAssertions Demo");
    
    runner.run_test("Basic Assertions", demo_basic_assertions);
    runner.run_test("Message Content Assertions", demo_message_content_assertions);
    runner.run_test("Log Level Assertions", demo_log_level_assertions);
    runner.run_test("Pattern Matching", demo_pattern_matching);
    runner.run_test("Custom Predicates", demo_custom_predicates);
    runner.run_test("Chronological Ordering", demo_chronological_ordering);
    runner.run_test("Performance Testing", demo_performance_testing);
    runner.run_test("Error Handling", demo_error_handling);
    runner.run_test("Comprehensive Scenario", demo_comprehensive_scenario);
    
    return 0;
}

/**
 * @brief Demonstrate basic buffer assertions
 */
void demo_basic_assertions() {
    TestScope test("BasicAssertions", 10, true);
    auto& logger = test.logger();
    auto assertions = test.assertions();
    
    // Test empty buffer
    assertions.assert_empty();
    assertions.assert_size(0);
    
    // Add some messages
    logger.info("First message");
    logger.debug("Second message");
    logger.error("Third message");
    
    // Test non-empty buffer
    assertions.assert_not_empty();
    assertions.assert_size(3);
    
    // Clear and test again
    assertions.clear_buffer();
    assertions.assert_empty();
    assertions.assert_size(0);
    
    std::cout << "✓ Basic assertions working correctly" << std::endl;
}

/**
 * @brief Demonstrate message content assertions
 */
void demo_message_content_assertions() {
    TestScope test("MessageContent", 20, true);
    auto& logger = test.logger();
    auto assertions = test.assertions();
    
    // Write specific messages
    logger.info("User authentication successful");
    logger.warn("Database connection unstable");
    logger.error("Failed to parse configuration file");
    logger.info("Application startup complete");
    
    // Test message content
    assertions.assert_contains_message("authentication");
    assertions.assert_contains_message("Database connection");
    assertions.assert_contains_message("configuration file");
    assertions.assert_not_contains_message("nonexistent content");
    
    // Test first and last messages
    assertions.assert_first_message_contains("authentication");
    assertions.assert_last_message_contains("startup complete");
    
    std::cout << "✓ Message content assertions working correctly" << std::endl;
}

/**
 * @brief Demonstrate log level assertions
 */
void demo_log_level_assertions() {
    TestScope test("LogLevels", 50, true);
    auto& logger = test.logger();
    auto assertions = test.assertions();
    
    // Generate messages with different levels
    TestDataGenerator::generate_mixed_level_messages(logger, "Test", 2);
    
    // Count messages by level (2 of each level)
    assertions.assert_level_count(LogLevel::TRACE, 2);
    assertions.assert_level_count(LogLevel::DEBUG, 2);
    assertions.assert_level_count(LogLevel::INFO, 2);
    assertions.assert_level_count(LogLevel::WARN, 2);
    assertions.assert_level_count(LogLevel::ERROR, 2);
    assertions.assert_level_count(LogLevel::FATAL, 0); // No FATAL messages
    
    // Total should be 10 (5 levels × 2 messages each)
    assertions.assert_size(10);
    
    std::cout << "✓ Log level assertions working correctly" << std::endl;
}

/**
 * @brief Demonstrate pattern matching with regex
 */
void demo_pattern_matching() {
    TestScope test("PatternMatching", 20, true);
    auto& logger = test.logger();
    auto assertions = test.assertions();
    
    // Write messages with specific patterns
    logger.info("User ID: 12345 logged in");
    logger.warn("Temperature: 75.5°C exceeds threshold");
    logger.error("Exception: java.lang.NullPointerException at line 42");
    logger.info("Processing file: document.pdf (size: 1.2MB)");
    
    // Test pattern matching
    assertions.assert_contains_pattern(R"(User ID: \d+ logged in)");
    assertions.assert_contains_pattern(R"(Temperature: \d+\.\d+°C)");
    assertions.assert_contains_pattern(R"(Exception: \w+\.\w+\.\w+ at line \d+)");
    assertions.assert_contains_pattern(R"(size: \d+\.\d+MB)");
    
    std::cout << "✓ Pattern matching assertions working correctly" << std::endl;
}

/**
 * @brief Demonstrate custom predicate assertions
 */
void demo_custom_predicates() {
    TestScope test("CustomPredicates", 30, true);
    auto& logger = test.logger();
    auto assertions = test.assertions();
    
    // Write messages with different lengths and characteristics
    logger.info("Short");
    logger.warn("This is a medium length message");
    logger.error("This is a very long message that contains many words and should be easily identifiable");
    logger.debug("Another short one");
    logger.info("Medium length message again");
    
    // Test custom predicates
    assertions.assert_any_match(
        [](const LogEntry& entry) { return entry.message.length() < 10; },
        "at least one short message (< 10 chars)"
    );
    
    assertions.assert_any_match(
        [](const LogEntry& entry) { return entry.message.length() > 50; },
        "at least one long message (> 50 chars)"
    );
    
    // Test that all messages contain some common pattern
    assertions.assert_all_match(
        [](const LogEntry& entry) { return !entry.message.empty(); },
        "all messages are non-empty"
    );
    
    // Test specific level conditions
    assertions.assert_any_match(
        [](const LogEntry& entry) { 
            return entry.level == LogLevel::ERROR && entry.message.find("very long") != std::string::npos; 
        },
        "at least one ERROR message containing 'very long'"
    );
    
    std::cout << "✓ Custom predicate assertions working correctly" << std::endl;
}

/**
 * @brief Demonstrate chronological ordering assertions
 */
void demo_chronological_ordering() {
    TestScope test("ChronologicalOrder", 20, true);
    auto& logger = test.logger();
    auto assertions = test.assertions();
    
    // Write messages with small delays to ensure different timestamps
    logger.info("First message");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    logger.warn("Second message");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    logger.error("Third message");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    logger.debug("Fourth message");
    
    // Test chronological ordering
    assertions.assert_chronological_order();
    assertions.assert_size(4);
    
    std::cout << "✓ Chronological ordering assertions working correctly" << std::endl;
}

/**
 * @brief Demonstrate performance testing capabilities
 */
void demo_performance_testing() {
    TestScope test("Performance", 1000, true);  // Larger buffer for performance test
    auto& logger = test.logger();
    auto assertions = test.assertions();
    
    const size_t message_count = 500;
    
    // Measure logging performance
    {
        PerformanceMeasurer perf("Logging " + std::to_string(message_count) + " messages");
        TestDataGenerator::generate_burst_messages(logger, message_count, "PerfTest");
    }
    
    // Verify all messages were logged
    assertions.assert_size(message_count);
    
    // Measure assertion performance
    {
        PerformanceMeasurer perf("Executing buffer assertions");
        assertions.assert_not_empty();
        assertions.assert_level_count(LogLevel::INFO, message_count);
        assertions.assert_contains_message("PerfTest message 0");
        assertions.assert_contains_message("PerfTest message " + std::to_string(message_count - 1));
        assertions.assert_chronological_order();
    }
    
    std::cout << "✓ Performance testing completed" << std::endl;
}

/**
 * @brief Demonstrate error handling in assertions
 */
void demo_error_handling() {
    TestScope test("ErrorHandling", 10, true);
    auto& logger = test.logger();
    auto assertions = test.assertions();
    
    // Test assertion failures
    bool caught_exception = false;
    
    try {
        // This should fail - buffer is empty
        assertions.assert_size(5);
    } catch (const BufferAssertionException& e) {
        caught_exception = true;
        std::cout << "Expected assertion failure caught: " << e.what() << std::endl;
    }
    
    if (!caught_exception) {
        throw std::runtime_error("Expected assertion exception was not thrown");
    }
    
    // Add a message and test another failure
    logger.info("Only message");
    caught_exception = false;
    
    try {
        // This should fail - looking for non-existent message
        assertions.assert_contains_message("non-existent message");
    } catch (const BufferAssertionException& e) {
        caught_exception = true;
        std::cout << "Expected assertion failure caught: " << e.what() << std::endl;
    }
    
    if (!caught_exception) {
        throw std::runtime_error("Expected assertion exception was not thrown");
    }
    
    std::cout << "✓ Error handling working correctly" << std::endl;
}

/**
 * @brief Comprehensive scenario demonstrating real-world usage
 */
void demo_comprehensive_scenario() {
    std::cout << "\n--- Comprehensive Testing Scenario ---" << std::endl;
    
    TestScope test("ComprehensiveTest", 100, false); // Keep console output for this demo
    auto& logger = test.logger();
    auto assertions = test.assertions();
    
    std::cout << "Simulating application startup and operation..." << std::endl;
    
    // Simulate application startup
    logger.info("Application starting...");
    logger.debug("Loading configuration from config.ini");
    logger.info("Database connection established");
    logger.warn("Using default settings for missing config values");
    
    // Simulate some operations
    TestDataGenerator::generate_formatted_messages(logger, 3);
    
    // Simulate an error condition
    logger.error("Failed to process user request: invalid input");
    logger.warn("Retrying operation with fallback method");
    logger.info("Operation completed successfully using fallback");
    
    // Simulate shutdown
    logger.info("Application shutting down...");
    logger.debug("Closing database connections");
    logger.info("Shutdown complete");
    
    std::cout << "\nValidating logged messages..." << std::endl;
    
    // Comprehensive validations
    assertions.assert_not_empty();
    std::cout << "Buffer contains " << assertions.get_buffer_size() << " entries" << std::endl;
    
    // Validate startup sequence
    assertions.assert_first_message_contains("Application starting");
    assertions.assert_contains_message("Database connection established");
    assertions.assert_contains_message("config.ini");
    
    // Validate error handling
    assertions.assert_level_count(LogLevel::ERROR, 4); // 3 from formatted messages + 1 specific error
    assertions.assert_contains_message("Failed to process user request");
    assertions.assert_contains_message("fallback method");
    
    // Validate shutdown sequence
    assertions.assert_last_message_contains("Shutdown complete");
    assertions.assert_contains_message("Closing database connections");
    
    // Validate message patterns
    assertions.assert_contains_pattern(R"(User user\d+ logged in at \d+)");
    assertions.assert_contains_pattern(R"(Error code: \d+, details: .+)");
    
    // Validate chronological order
    assertions.assert_chronological_order();
    
    // Custom validation: ensure no FATAL messages (application didn't crash)
    assertions.assert_level_count(LogLevel::FATAL, 0);
    
    // Custom validation: ensure balanced startup/shutdown
    assertions.assert_any_match(
        [](const LogEntry& entry) { return entry.message.find("starting") != std::string::npos; },
        "contains startup message"
    );
    assertions.assert_any_match(
        [](const LogEntry& entry) { return entry.message.find("shutting down") != std::string::npos; },
        "contains shutdown message"
    );
    
    // Print buffer contents for inspection
    std::cout << "\nFinal buffer contents:" << std::endl;
    assertions.print_buffer_contents();
    
    std::cout << "\n✓ Comprehensive scenario validation completed successfully" << std::endl;
}
