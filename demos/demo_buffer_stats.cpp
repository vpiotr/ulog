/**
 * @file demo_buffer_stats.cpp
 * @brief Demonstration of BufferStats extension for log buffer performance analysis
 * @author ulog team
 * @version 1.0.0
 * 
 * This demo shows how to use the BufferStats extension to analyze log buffer
 * contents for performance metrics, outlier detection, and pattern analysis.
 * It simulates realistic application scenarios with various types of operations
 * and their characteristic timing patterns.
 * 
 * Key features demonstrated:
 * - Statistical analysis of log buffer contents
 * - Performance metrics for slow operations (SQL, AWS, etc.)
 * - Lock contention pattern detection
 * - Outlier identification and analysis
 * - Comprehensive reporting with insights
 * - Realistic simulation of batch processing scenarios
 * 
 * Compilation:
 * g++ -std=c++17 -I../include -I../demos/include -I../demos/extensions/include \
 *     demo_buffer_stats.cpp -o demo_buffer_stats -pthread
 */

#include "ulog/ulog.h"
#include "buffer_stats.h"
#include "buffer_stats_reporter.h"
#include "test_helpers.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <algorithm>

using namespace ulog;
using namespace ulog::extensions;
using namespace ulog::demo;

// Forward declarations for simulation functions
void simulate_sql_operations(Logger& logger, std::mt19937& rng);
void simulate_aws_operations(Logger& logger, std::mt19937& rng);
void simulate_lock_contention(Logger& logger, std::mt19937& rng);
void simulate_normal_operations(Logger& logger, std::mt19937& rng);
void simulate_outlier_scenarios(Logger& logger, std::mt19937& rng);
void simulate_batch_processing(Logger& logger);

// Demo functions
void demo_basic_statistics();
void demo_timing_analysis();
void demo_pattern_analysis();
void demo_outlier_detection();
void demo_performance_analysis();
void demo_contention_analysis();
void demo_lambda_filtering();
void demo_comprehensive_scenario();

int main() {
    std::cout << "=== BufferStats Demo ===" << std::endl;
    std::cout << "This demo shows how to use BufferStats for analyzing log buffer performance." << std::endl;
    std::cout << std::endl;
    
    // Run all demonstrations
    SimpleTestRunner runner("BufferStats Demo");
    
    runner.run_test("Basic Statistics", demo_basic_statistics);
    runner.run_test("Timing Analysis", demo_timing_analysis);
    runner.run_test("Pattern Analysis", demo_pattern_analysis);
    runner.run_test("Outlier Detection", demo_outlier_detection);
    runner.run_test("Performance Analysis", demo_performance_analysis);
    runner.run_test("Contention Analysis", demo_contention_analysis);
    runner.run_test("Lambda Filtering", demo_lambda_filtering);
    runner.run_test("Comprehensive Scenario", demo_comprehensive_scenario);
    
    return 0;
}

/**
 * @brief Demonstrate basic buffer statistics
 */
void demo_basic_statistics() {
    TestScope test("BasicStats", 100, true);
    auto& logger = test.logger();
    
    // Generate mixed messages
    logger.info("Application started");
    logger.debug("Loading configuration");
    logger.info("Configuration loaded successfully");
    logger.warn("Deprecated API used");
    logger.error("Connection failed, retrying");
    logger.info("Connection established");
    logger.debug("Processing request");
    logger.info("Request completed");
    
    BufferStats stats(logger);
    
    std::cout << "Total messages: " << stats.total_messages() << std::endl;
    std::cout << "INFO messages: " << stats.messages_by_level(LogLevel::INFO) << std::endl;
    std::cout << "DEBUG messages: " << stats.messages_by_level(LogLevel::DEBUG) << std::endl;
    std::cout << "WARN messages: " << stats.messages_by_level(LogLevel::WARN) << std::endl;
    std::cout << "ERROR messages: " << stats.messages_by_level(LogLevel::ERROR) << std::endl;
    
    auto distribution = stats.level_distribution();
    std::cout << "Level distribution:" << std::endl;
    for (const auto& pair : distribution) {
        std::cout << "  " << to_string(pair.first) << ": " << pair.second << std::endl;
    }
    
    std::cout << "✓ Basic statistics working correctly" << std::endl;
}

/**
 * @brief Demonstrate timing analysis capabilities
 */
void demo_timing_analysis() {
    TestScope test("TimingAnalysis", 50, true);
    auto& logger = test.logger();
    
    // Generate messages with varying intervals
    logger.info("Fast operation 1");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    logger.info("Fast operation 2");
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    
    logger.info("Slow operation");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    logger.info("Fast operation 3");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    logger.info("Very slow operation");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    logger.info("Final operation");
    
    BufferStats stats(logger);
    
    std::cout << "Total timespan: " << stats.total_timespan().count() << " ms" << std::endl;
    std::cout << "Average interval: " << stats.average_interval().count() << " ms" << std::endl;
    std::cout << "Median interval: " << stats.median_interval().count() << " ms" << std::endl;
    
    auto intervals = stats.all_intervals();
    std::cout << "All intervals: ";
    for (const auto& interval : intervals) {
        std::cout << interval.count() << "ms ";
    }
    std::cout << std::endl;
    
    std::cout << "✓ Timing analysis working correctly" << std::endl;
}

/**
 * @brief Demonstrate pattern analysis features
 */
void demo_pattern_analysis() {
    TestScope test("PatternAnalysis", 100, true);
    auto& logger = test.logger();
    
    // Generate messages with repeating patterns
    for (int i = 0; i < 5; ++i) {
        logger.info("SQL_SELECT: fetching user data");
        logger.info("CACHE_HIT: user data found");
        logger.info("TRANSFORM: applying business rules");
    }
    
    for (int i = 0; i < 3; ++i) {
        logger.info("AWS_S3: uploading file");
        logger.info("NOTIFY: sending email");
    }
    
    logger.info("ERROR_RECOVERY: retrying operation");
    logger.info("ERROR_RECOVERY: operation successful");
    
    BufferStats stats(logger);
    
    auto top_prefixes = stats.top_prefixes(5, 15);
    std::cout << "Top prefixes:" << std::endl;
    for (const auto& pair : top_prefixes) {
        std::cout << "  \"" << pair.first << "\": " << pair.second << " occurrences" << std::endl;
    }
    
    auto frequent_messages = stats.message_frequency(3);
    std::cout << "Most frequent messages:" << std::endl;
    for (const auto& pair : frequent_messages) {
        std::cout << "  \"" << pair.first << "\": " << pair.second << " times" << std::endl;
    }
    
    std::cout << "✓ Pattern analysis working correctly" << std::endl;
}

/**
 * @brief Demonstrate outlier detection capabilities
 */
void demo_outlier_detection() {
    TestScope test("OutlierDetection", 50, true);
    auto& logger = test.logger();
    
    // Generate normal operations with consistent timing
    for (int i = 0; i < 10; ++i) {
        logger.info("Normal operation " + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    // Create outliers
    logger.info("Outlier operation 1");
    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Outlier
    
    logger.info("Normal operation continues");
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    
    logger.info("Extreme outlier operation");
    std::this_thread::sleep_for(std::chrono::milliseconds(800)); // Extreme outlier
    
    logger.info("Back to normal");
    
    BufferStats stats(logger);
    
    auto outliers_90 = stats.delay_outliers(90.0);
    std::cout << "90th percentile outliers (" << outliers_90.size() << " found):" << std::endl;
    for (const auto& outlier : outliers_90) {
        std::cout << "  #" << outlier.message_index << " - " << outlier.delay_after.count() 
                  << "ms delay after: \"" << outlier.message_prefix << "\"" << std::endl;
    }
    
    auto outliers_95 = stats.delay_outliers(95.0);
    std::cout << "95th percentile outliers: " << outliers_95.size() << std::endl;
    
    std::cout << "✓ Outlier detection working correctly" << std::endl;
}

/**
 * @brief Demonstrate performance analysis features
 */
void demo_performance_analysis() {
    TestScope test("PerformanceAnalysis", 100, true);
    auto& logger = test.logger();
    
    std::mt19937 rng(42); // Fixed seed for reproducible results
    
    // Simulate various operations
    simulate_sql_operations(logger, rng);
    simulate_aws_operations(logger, rng);
    simulate_normal_operations(logger, rng);
    
    BufferStats stats(logger);
    
    // Analyze slow operations
    std::vector<std::string> slow_prefixes = {"SQL_", "AWS_", "DB_"};
    auto perf_metrics = stats.analyze_slow_operations(slow_prefixes);
    
    std::cout << "Performance Metrics:" << std::endl;
    std::cout << "  Slow operations count: " << perf_metrics.slow_operations_count << std::endl;
    std::cout << "  Slowest operation: " << perf_metrics.slowest_operation.count() << " ms" << std::endl;
    std::cout << "  Average slow operation: " << perf_metrics.average_slow_operation.count() << " ms" << std::endl;
    
    std::cout << "Top slow operations:" << std::endl;
    size_t count = 0;
    for (const auto& op : perf_metrics.slow_operations) {
        if (count >= 5) break;
        std::cout << "  " << op.second.count() << "ms - " << op.first << std::endl;
        count++;
    }
    
    std::cout << "✓ Performance analysis working correctly" << std::endl;
}

/**
 * @brief Demonstrate contention analysis features
 */
void demo_contention_analysis() {
    TestScope test("ContentionAnalysis", 100, true);
    auto& logger = test.logger();
    
    std::mt19937 rng(123); // Fixed seed for reproducible results
    
    // Simulate lock contention scenarios
    simulate_lock_contention(logger, rng);
    simulate_normal_operations(logger, rng);
    
    BufferStats stats(logger);
    
    // Analyze contention patterns
    std::vector<std::string> contention_prefixes = {"LOCK_", "MUTEX_", "SYNC_"};
    auto contention_metrics = stats.analyze_contention(contention_prefixes);
    
    std::cout << "Contention Metrics:" << std::endl;
    std::cout << "  Variable delay operations: " << contention_metrics.variable_delay_count << std::endl;
    std::cout << "  Max delay: " << contention_metrics.max_delay.count() << " ms" << std::endl;
    std::cout << "  Min delay: " << contention_metrics.min_delay.count() << " ms" << std::endl;
    std::cout << "  Average delay: " << contention_metrics.average_delay.count() << " ms" << std::endl;
    
    std::cout << "Contention patterns:" << std::endl;
    for (const auto& pattern : contention_metrics.contention_patterns) {
        if (!pattern.second.empty()) {
            auto sum = std::accumulate(pattern.second.begin(), pattern.second.end(), 
                                     std::chrono::milliseconds(0));
            auto avg = sum / pattern.second.size();
            std::cout << "  " << pattern.first << ": " << pattern.second.size() 
                      << " ops, avg=" << avg.count() << "ms" << std::endl;
        }
    }
    
    std::cout << "✓ Contention analysis working correctly" << std::endl;
}

/**
 * @brief Demonstrate lambda-based filtering capabilities
 */
void demo_lambda_filtering() {
    TestScope test("LambdaFiltering", 50, true);
    auto& logger = test.logger();
    
    // Generate mixed messages
    logger.info("User login successful");
    logger.error("Database connection failed");
    logger.info("SQL_SELECT: user profile");
    logger.warn("Deprecated function called");
    logger.info("AWS_S3: file uploaded");
    logger.error("Validation failed");
    logger.info("Cache miss for key: user123");
    
    BufferStats stats(logger);
    
    // Filter error messages
    auto error_messages = stats.filter_messages([](const LogEntry& entry) {
        return entry.level == LogLevel::ERROR;
    });
    
    std::cout << "Error messages (" << error_messages.size() << " found):" << std::endl;
    for (const auto& entry : error_messages) {
        std::cout << "  [" << to_string(entry.level) << "] " << entry.message << std::endl;
    }
    
    // Filter messages containing "SQL" or "AWS"
    auto cloud_db_messages = stats.filter_messages([](const LogEntry& entry) {
        return entry.message.find("SQL") != std::string::npos || 
               entry.message.find("AWS") != std::string::npos;
    });
    
    std::cout << "Cloud/DB messages (" << cloud_db_messages.size() << " found):" << std::endl;
    for (const auto& entry : cloud_db_messages) {
        std::cout << "  " << entry.message << std::endl;
    }
    
    // Get longest messages
    auto longest_messages = stats.top_messages_by(3, 
        [](const LogEntry& entry) { return entry.message.length(); });
    
    std::cout << "Longest messages:" << std::endl;
    for (const auto& entry : longest_messages) {
        std::cout << "  (" << entry.message.length() << " chars) " << entry.message << std::endl;
    }
    
    std::cout << "✓ Lambda filtering working correctly" << std::endl;
}

/**
 * @brief Demonstrate comprehensive scenario with full reporting
 */
void demo_comprehensive_scenario() {
    TestScope test("ComprehensiveScenario", 500, true);
    auto& logger = test.logger();
    
    std::cout << "Simulating comprehensive batch processing scenario..." << std::endl;
    
    // Simulate a complex batch processing scenario
    simulate_batch_processing(logger);
    
    std::cout << "Simulation completed. Analyzing results..." << std::endl;
    
    // Perform comprehensive analysis
    BufferStats stats(logger);
    BufferStatsReporter reporter(stats);
    
    // Generate and display full report
    std::cout << std::endl;
    std::cout << "=== COMPREHENSIVE ANALYSIS REPORT ===" << std::endl;
    std::cout << reporter.generate_full_report() << std::endl;
    
    // Save report to file for further analysis
    try {
        reporter.save_report("demo_buffer_stats_report.txt");
        std::cout << "Report saved to: demo_buffer_stats_report.txt" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Could not save report to file: " << e.what() << std::endl;
    }
    
    std::cout << "✓ Comprehensive scenario analysis completed" << std::endl;
}

// Simulation Functions

/**
 * @brief Simulate SQL database operations with realistic delays
 */
void simulate_sql_operations(Logger& logger, std::mt19937& rng) {
    std::uniform_int_distribution<> delay_dist(20, 80); // Reduced from 100-500 to 20-80ms
    std::vector<std::string> sql_ops = {
        "SQL_SELECT: user data query",
        "SQL_INSERT: new record creation",
        "SQL_UPDATE: profile modification",
        "SQL_DELETE: cleanup operation",
        "SQL_BATCH: bulk data processing"
    };
    
    for (int i = 0; i < 5; ++i) { // Reduced from 8 to 5 operations
        auto op = sql_ops[rng() % sql_ops.size()];
        logger.info(op + " #" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(rng)));
    }
}

/**
 * @brief Simulate AWS cloud operations with variable delays
 */
void simulate_aws_operations(Logger& logger, std::mt19937& rng) {
    std::uniform_int_distribution<> delay_dist(30, 100); // Reduced from 200-800 to 30-100ms
    std::vector<std::string> aws_ops = {
        "AWS_S3: file upload operation",
        "AWS_LAMBDA: function invocation",
        "AWS_RDS: database connection",
        "AWS_SQS: message queue operation",
        "AWS_EC2: instance management"
    };
    
    for (int i = 0; i < 4; ++i) { // Reduced from 6 to 4 operations
        auto op = aws_ops[rng() % aws_ops.size()];
        logger.info(op + " #" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(rng)));
    }
}

/**
 * @brief Simulate lock contention with highly variable delays
 */
void simulate_lock_contention(Logger& logger, std::mt19937& rng) {
    std::uniform_int_distribution<> delay_dist(1, 200); // Reduced from 1-5000 to 1-200ms
    std::vector<std::string> lock_ops = {
        "LOCK_ACQUIRE: obtaining resource lock",
        "MUTEX_WAIT: waiting for mutex",
        "SYNC_BARRIER: synchronization point",
        "LOCK_RELEASE: releasing resource",
        "CONDITION_WAIT: waiting for condition"
    };
    
    for (int i = 0; i < 6; ++i) { // Reduced from 10 to 6 operations
        auto op = lock_ops[rng() % lock_ops.size()];
        logger.info(op + " #" + std::to_string(i));
        
        // Simulate contention with variable delays
        auto delay = delay_dist(rng);
        if (delay > 3000) {
            // Simulate severe contention
            delay = delay + 2000;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

/**
 * @brief Simulate normal fast operations
 */
void simulate_normal_operations(Logger& logger, std::mt19937& rng) {
    std::uniform_int_distribution<> delay_dist(1, 20); // Reduced from 1-50 to 1-20ms
    std::vector<std::string> normal_ops = {
        "CACHE_HIT: data found in cache",
        "VALIDATION: input validation passed",
        "TRANSFORM: data transformation",
        "NOTIFY: sending notification",
        "LOG_ROTATION: log file rotation"
    };
    
    for (int i = 0; i < 8; ++i) { // Reduced from 15 to 8 operations
        auto op = normal_ops[rng() % normal_ops.size()];
        logger.info(op + " #" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(rng)));
    }
}

/**
 * @brief Simulate outlier scenarios with extreme delays
 */
void simulate_outlier_scenarios(Logger& logger, std::mt19937& rng) {
    std::uniform_int_distribution<> normal_delay(5, 15); // Reduced from 10-50 to 5-15ms
    std::vector<std::string> outlier_ops = {
        "MEMORY_ALLOCATION: large buffer allocation",
        "NETWORK_TIMEOUT: external service timeout",
        "DISK_IO: large file operation",
        "GARBAGE_COLLECTION: full GC cycle"
    };
    
    // Normal operations
    for (int i = 0; i < 3; ++i) { // Reduced from 5 to 3 operations
        logger.info("NORMAL_OP: regular processing #" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(normal_delay(rng)));
    }
    
    // Outlier operation
    auto outlier_op = outlier_ops[rng() % outlier_ops.size()];
    logger.warn(outlier_op + " - may take longer");
    std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Reduced from 2500 to 150ms
    
    // More normal operations
    for (int i = 0; i < 2; ++i) { // Reduced from 3 to 2 operations
        logger.info("NORMAL_OP: continuing processing #" + std::to_string(i + 3));
        std::this_thread::sleep_for(std::chrono::milliseconds(normal_delay(rng)));
    }
    
    // Another outlier
    logger.error("SYSTEM_RECOVERY: recovering from error state");
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Reduced from 1800 to 100ms
    
    logger.info("SYSTEM_RECOVERY: recovery completed");
}

/**
 * @brief Simulate comprehensive batch processing scenario
 */
void simulate_batch_processing(Logger& logger) {
    std::mt19937 rng(12345); // Fixed seed for reproducible results
    
    logger.info("BATCH_START: Beginning batch processing job");
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Reduced from 50ms
    
    // Initialization phase
    logger.info("INIT: Loading configuration");
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Reduced from 100ms
    
    logger.info("INIT: Connecting to databases");
    std::this_thread::sleep_for(std::chrono::milliseconds(30)); // Reduced from 300ms
    
    logger.info("INIT: Validating input data");
    std::this_thread::sleep_for(std::chrono::milliseconds(15)); // Reduced from 75ms
    
    // Main processing with mixed operations - reduced from 3 to 2 batches
    for (int batch = 1; batch <= 2; ++batch) {
        logger.info("BATCH_PHASE: Starting batch " + std::to_string(batch));
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Reduced from 25ms
        
        // Simulate SQL operations
        simulate_sql_operations(logger, rng);
        
        // Simulate AWS operations
        simulate_aws_operations(logger, rng);
        
        // Simulate some contention
        simulate_lock_contention(logger, rng);
        
        // Simulate normal operations
        simulate_normal_operations(logger, rng);
        
        // Simulate outliers
        simulate_outlier_scenarios(logger, rng);
        
        logger.info("BATCH_PHASE: Completed batch " + std::to_string(batch));
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Reduced from 100ms
    }
    
    // Cleanup phase
    logger.info("CLEANUP: Finalizing transactions");
    std::this_thread::sleep_for(std::chrono::milliseconds(25)); // Reduced from 200ms
    
    logger.info("CLEANUP: Closing connections");
    std::this_thread::sleep_for(std::chrono::milliseconds(15)); // Reduced from 150ms
    
    logger.info("BATCH_END: Batch processing completed successfully");
}
