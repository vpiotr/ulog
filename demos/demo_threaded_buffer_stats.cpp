/**
 * @file demo_threaded_buffer_stats.cpp
 * @brief Multi-threaded demonstration of BufferStats with per-thread analysis
 * @author ulog team
 * @version 1.0.0
 * 
 * This demo shows how to use the enhanced ThreadBufferAnalyzer and 
 * MultiThreadReporter extensions to analyze log messages from multiple threads.
 * It demonstrates a clean and simple approach while the extensions handle complexity.
 * 
 * Compilation:
 * g++ -std=c++17 -I../include -I../demos/include -I../demos/extensions/include \
 *     demo_threaded_buffer_stats.cpp -o demo_threaded_buffer_stats -pthread
 */

#include "ulog/ulog.h"
#include "thread_aware_logger.h"
#include "thread_buffer_analyzer.h"
#include "multi_thread_reporter.h"
#include "threaded_work_simulator.h"
#include "simple_console_observer.h"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <memory>

using namespace ulog;
using namespace ulog::extensions;
using namespace ulog::demo;

/**
 * @brief Main demonstration function
 */
void demo_multi_threaded_analysis() {
    std::cout << "=== Multi-Threaded Buffer Stats Demo ===\n\n";
    
    // 1. Setup logger with thread-aware wrapper and large buffer
    auto& base_logger = getLogger("MultiThreadApp");
    base_logger.enable_buffer(10000);
    base_logger.set_log_level(LogLevel::TRACE);
    
    // Add console observer to base logger
    auto console_observer = std::make_shared<SimpleConsoleObserver>();
    base_logger.add_observer(console_observer);
    
    // Create thread-aware logger wrapper
    ThreadAwareLogger thread_logger(base_logger);
    
    std::cout << "Logger configured with thread-aware wrapper and buffer size 10000\n\n";
    
    // 2. Record start time
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 3. Launch worker threads with different characteristics
    std::vector<std::thread> workers;
    
    // Database workers - slow operations
    workers.emplace_back([&thread_logger] {
        WorkerConfig config{
            "DatabaseWorker",
            8,
            std::chrono::milliseconds(100),
            std::chrono::milliseconds(200),
            {"SQL_SELECT", "SQL_INSERT", "SQL_UPDATE", "CONN_POOL"}
        };
        ThreadedWorkSimulator::simulateDatabaseWork(thread_logger, config);
    });
    
    // Web handlers - fast operations with occasional errors
    workers.emplace_back([&thread_logger] {
        WorkerConfig config{
            "WebHandler",
            15,
            std::chrono::milliseconds(50),
            std::chrono::milliseconds(100),
            {"HTTP_GET", "HTTP_POST", "AUTH_CHECK", "CACHE_LOOKUP"}
        };
        ThreadedWorkSimulator::simulateWebWork(thread_logger, config);
    });
    
    // Background processor - medium speed operations
    workers.emplace_back([&thread_logger] {
        WorkerConfig config{
            "BackgroundProcessor",
            10,
            std::chrono::milliseconds(80),
            std::chrono::milliseconds(120),
            {"FILE_PROCESS", "EMAIL_SEND", "CLEANUP_TASK"}
        };
        ThreadedWorkSimulator::simulateBackgroundWork(thread_logger, config);
    });
    
    // System monitor - frequent, fast operations
    workers.emplace_back([&thread_logger] {
        WorkerConfig config{
            "SystemMonitor",
            20,
            std::chrono::milliseconds(25),
            std::chrono::milliseconds(50),
            {"HEALTH_CHECK", "METRICS_COLLECT", "DISK_CHECK"}
        };
        ThreadedWorkSimulator::simulateMonitoringWork(thread_logger, config);
    });
    
    std::cout << "Started 4 worker threads with different operation patterns\n\n";
    
    // 4. Wait for all workers to complete
    std::cout << "Waiting for all workers to complete...\n";
    for (auto& worker : workers) {
        worker.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "\nAll workers completed. Starting analysis...\n";
    
    // 5. Analyze results by thread
    ThreadBufferAnalyzer analyzer;
    auto thread_entries = analyzer.analyzeByThread(*thread_logger.buffer());
    
    std::cout << "Organized " << thread_logger.buffer()->size() 
              << " log entries into " << thread_entries.size() << " threads\n\n";
    
    // 6. Generate and print comprehensive report
    MultiThreadReporter reporter;
    auto overall_report = reporter.generateReport(thread_entries);
    overall_report.total_execution_time = execution_time;
    
    reporter.printReport(overall_report);
    
    // 7. Demonstrate filtering capabilities
    std::cout << "FILTERED ANALYSIS (Errors Only):\n";
    auto error_predicate = [](const LogEntry& entry) {
        return entry.level == LogLevel::ERROR || entry.level == LogLevel::FATAL;
    };
    
    auto error_entries = analyzer.analyzeByThread(*thread_logger.buffer(), error_predicate);
    auto error_report = reporter.generateReport(error_entries);
    
    std::cout << "Found " << error_report.total_log_entries 
              << " error entries across " << error_report.total_threads << " threads\n\n";
}

/**
 * @brief Demonstration of thread distribution analysis
 */
void demo_thread_distribution() {
    std::cout << "=== Thread Distribution Analysis ===\n\n";
    
    auto& base_logger = getLogger("DistributionTest");
    base_logger.enable_buffer(1000);
    
    // Setup console observer
    auto console_observer = std::make_shared<SimpleConsoleObserver>();
    base_logger.add_observer(console_observer);
    
    // Create thread-aware logger wrapper
    ThreadAwareLogger thread_logger(base_logger);
    
    // Create 3 threads with different message counts
    std::vector<std::thread> threads;
    
    threads.emplace_back([&thread_logger] {
        for (int i = 0; i < 5; ++i) {
            thread_logger.info("Low activity thread message {0}", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    threads.emplace_back([&thread_logger] {
        for (int i = 0; i < 15; ++i) {
            thread_logger.info("High activity thread message {0}", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });
    
    threads.emplace_back([&thread_logger] {
        for (int i = 0; i < 10; ++i) {
            thread_logger.info("Medium activity thread message {0}", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    });
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Analyze distribution
    ThreadBufferAnalyzer analyzer;
    auto thread_entries = analyzer.analyzeByThread(*thread_logger.buffer());
    auto stats = analyzer.getDistributionStats(thread_entries);
    
    std::cout << "Thread Distribution Statistics:\n";
    std::cout << "  Total Threads: " << stats.total_threads << "\n";
    std::cout << "  Total Entries: " << stats.total_entries << "\n";
    std::cout << "  Average Entries per Thread: " << stats.avg_entries_per_thread << "\n";
    std::cout << "  Most Active Thread: " << stats.most_active_thread.substr(0, 12) << "... (" 
              << stats.max_entries_per_thread << " entries)\n";
    std::cout << "  Least Active Thread: " << stats.least_active_thread.substr(0, 12) << "... (" 
              << stats.min_entries_per_thread << " entries)\n\n";
}

/**
 * @brief Main function
 */
int main() {
    try {
        demo_multi_threaded_analysis();
        demo_thread_distribution();
        
        std::cout << "Demo completed successfully!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Demo failed with error: " << e.what() << std::endl;
        return 1;
    }
}
