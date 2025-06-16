#ifndef ULOG_MULTI_THREAD_REPORTER_H
#define ULOG_MULTI_THREAD_REPORTER_H

/**
 * @file multi_thread_reporter.h
 * @brief Multi-threaded log analysis reporter
 * @author ulog team
 * @version 1.0.0
 * 
 * This extension provides comprehensive reporting for multi-threaded log analysis,
 * generating per-thread statistics and aggregated reports.
 */

#include "ulog/ulog.h"
#include "thread_buffer_analyzer.h"
#include "buffer_stats.h"
#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <ctime>

namespace ulog {
namespace extensions {

/**
 * @brief Detailed slow operation information
 */
struct SlowOperation {
    std::chrono::system_clock::time_point timestamp; ///< When the operation started
    LogLevel level;                                   ///< Log level
    std::string thread_id;                           ///< Thread identifier
    std::string prefix;                              ///< Operation prefix (e.g., SQL_SELECT)
    std::string full_message;                        ///< Complete log message
    std::chrono::milliseconds duration;              ///< Operation duration
};

/**
 * @brief Report data for a single thread
 */
struct ThreadReport {
    std::string thread_id;                          ///< Thread identifier
    size_t message_count;                           ///< Number of messages from this thread
    std::chrono::milliseconds total_duration;       ///< Time span from first to last message
    std::chrono::milliseconds avg_message_interval; ///< Average time between messages
    size_t error_count;                             ///< Number of error/fatal messages
    std::vector<std::string> top_message_prefixes;  ///< Most common message prefixes
    size_t slow_operations_count;                   ///< Number of slow operations
    std::chrono::milliseconds slowest_operation;    ///< Duration of slowest operation
    size_t outlier_count;                           ///< Number of delay outliers
    double error_rate;                              ///< Error rate as percentage
    std::vector<SlowOperation> slow_operations;     ///< Detailed slow operations
};

/**
 * @brief Overall report containing all thread reports and summary
 */
struct OverallReport {
    std::chrono::milliseconds total_execution_time; ///< Total execution time
    size_t total_threads;                           ///< Number of threads
    size_t total_log_entries;                       ///< Total number of log entries
    std::vector<ThreadReport> thread_reports;       ///< Per-thread reports
    ThreadReport aggregated_stats;                  ///< Aggregated statistics
    std::vector<SlowOperation> all_slow_operations; ///< All slow operations across threads
};

/**
 * @brief Multi-threaded log analysis reporter
 * 
 * This class generates comprehensive reports for multi-threaded log analysis,
 * providing both per-thread statistics and overall aggregated metrics.
 * 
 * @code{.cpp}
 * ThreadBufferAnalyzer analyzer;
 * auto thread_entries = analyzer.analyzeByThread(logger.buffer());
 * 
 * MultiThreadReporter reporter;
 * auto report = reporter.generateReport(thread_entries);
 * reporter.printReport(report);
 * @endcode
 */
class MultiThreadReporter {
public:
    /**
     * @brief Generate comprehensive report from thread-organized entries
     * @param thread_entries Map of thread ID to log entries
     * @return Complete report with per-thread and aggregated statistics
     */
    OverallReport generateReport(const ThreadBufferAnalyzer::ThreadEntryMap& thread_entries) {
        OverallReport report{};
        
        // Calculate overall statistics
        report.total_threads = thread_entries.size();
        report.total_log_entries = 0;
        report.total_execution_time = std::chrono::milliseconds(0);
        
        std::chrono::system_clock::time_point earliest_time = std::chrono::system_clock::time_point::max();
        std::chrono::system_clock::time_point latest_time = std::chrono::system_clock::time_point::min();
        
        // Generate per-thread reports
        for (const auto& thread_pair : thread_entries) {
            ThreadReport thread_report = generateThreadReport(thread_pair.first, thread_pair.second);
            report.thread_reports.push_back(thread_report);
            report.total_log_entries += thread_report.message_count;
            
            // Collect all slow operations
            for (const auto& slow_op : thread_report.slow_operations) {
                report.all_slow_operations.push_back(slow_op);
            }
            
            // Track earliest and latest times for overall duration
            if (!thread_pair.second.empty()) {
                auto first_time = thread_pair.second.front().timestamp;
                auto last_time = thread_pair.second.back().timestamp;
                
                if (first_time < earliest_time) earliest_time = first_time;
                if (last_time > latest_time) latest_time = last_time;
            }
        }
        
        // Calculate total execution time
        if (earliest_time != std::chrono::system_clock::time_point::max()) {
            report.total_execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                latest_time - earliest_time);
        }
        
        // Generate aggregated statistics
        report.aggregated_stats = generateAggregatedStats(report.thread_reports);
        
        return report;
    }
    
    /**
     * @brief Print formatted report to console
     * @param report Report to print
     */
    void printReport(const OverallReport& report) {
        printHeader();
        printOverallSummary(report);
        printThreadSummaries(report.thread_reports);
        printAggregatedStats(report.aggregated_stats);
        printSlowOperationsTable(report.all_slow_operations);
        printFooter();
    }

private:
    /**
     * @brief Generate report for a single thread
     * @param thread_id Thread identifier
     * @param entries Log entries for this thread
     * @return Thread-specific report
     */
    ThreadReport generateThreadReport(const std::string& thread_id, 
                                    const std::vector<LogEntry>& entries) {
        ThreadReport report{};
        report.thread_id = thread_id;
        report.message_count = entries.size();
        report.error_count = 0;
        report.slow_operations_count = 0;
        report.slowest_operation = std::chrono::milliseconds(0);
        report.outlier_count = 0;
        
        if (entries.empty()) {
            report.error_rate = 0.0;
            return report;
        }
        
        // Calculate duration and average interval
        auto first_time = entries.front().timestamp;
        auto last_time = entries.back().timestamp;
        report.total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            last_time - first_time);
        
        if (entries.size() > 1) {
            report.avg_message_interval = std::chrono::milliseconds(
                report.total_duration.count() / (entries.size() - 1));
        }
        
        // Analyze entries for detailed statistics
        std::unordered_map<std::string, size_t> prefix_counts;
        std::vector<std::chrono::milliseconds> intervals;
        
        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            
            // Count errors
            if (entry.level == LogLevel::ERROR || entry.level == LogLevel::FATAL) {
                report.error_count++;
            }
            
            // Extract message prefix and count
            std::string prefix = extractMessagePrefix(entry.message);
            prefix_counts[prefix]++;
            
            // Calculate intervals for outlier detection
            if (i > 0) {
                auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(
                    entry.timestamp - entries[i-1].timestamp);
                intervals.push_back(interval);
                
                // Check for slow operations (SQL, AWS, etc.)
                if (isSlowOperationPrefix(prefix) && interval > std::chrono::milliseconds(100)) {
                    report.slow_operations_count++;
                    if (interval > report.slowest_operation) {
                        report.slowest_operation = interval;
                    }
                    
                    // Collect detailed slow operation information
                    SlowOperation slow_op;
                    slow_op.timestamp = entries[i-1].timestamp; // When the operation started
                    slow_op.level = entry.level;
                    slow_op.thread_id = thread_id;
                    slow_op.prefix = prefix;
                    slow_op.full_message = entry.message;
                    slow_op.duration = interval;
                    report.slow_operations.push_back(slow_op);
                }
            }
        }
        
        // Calculate error rate
        report.error_rate = (entries.size() > 0) ? 
            (static_cast<double>(report.error_count) / entries.size() * 100.0) : 0.0;
        
        // Detect outliers (intervals significantly larger than 90th percentile)
        if (!intervals.empty()) {
            std::sort(intervals.begin(), intervals.end());
            size_t p90_index = static_cast<size_t>(intervals.size() * 0.9);
            auto p90_threshold = intervals[std::min(p90_index, intervals.size() - 1)];
            
            for (const auto& interval : intervals) {
                if (interval > p90_threshold * 2) {  // 2x the 90th percentile
                    report.outlier_count++;
                }
            }
        }
        
        // Get top 3 message prefixes
        std::vector<std::pair<std::string, size_t>> sorted_prefixes(
            prefix_counts.begin(), prefix_counts.end());
        std::sort(sorted_prefixes.begin(), sorted_prefixes.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        for (size_t i = 0; i < std::min(size_t(3), sorted_prefixes.size()); ++i) {
            report.top_message_prefixes.push_back(sorted_prefixes[i].first);
        }
        
        return report;
    }
    
    /**
     * @brief Generate aggregated statistics across all threads
     * @param thread_reports Individual thread reports
     * @return Aggregated statistics
     */
    ThreadReport generateAggregatedStats(const std::vector<ThreadReport>& thread_reports) {
        ThreadReport aggregated{};
        aggregated.thread_id = "ALL";
        
        if (thread_reports.empty()) {
            return aggregated;
        }
        
        // Sum up basic stats
        for (const auto& report : thread_reports) {
            aggregated.message_count += report.message_count;
            aggregated.error_count += report.error_count;
            aggregated.slow_operations_count += report.slow_operations_count;
            aggregated.outlier_count += report.outlier_count;
            aggregated.total_duration = std::max(aggregated.total_duration, report.total_duration);
            aggregated.slowest_operation = std::max(aggregated.slowest_operation, report.slowest_operation);
        }
        
        // Calculate error rate
        aggregated.error_rate = (aggregated.message_count > 0) ? 
            (static_cast<double>(aggregated.error_count) / aggregated.message_count * 100.0) : 0.0;
        
        // Calculate average interval across all threads
        size_t total_intervals = 0;
        std::chrono::milliseconds total_interval_time(0);
        for (const auto& report : thread_reports) {
            if (report.message_count > 1) {
                total_intervals += (report.message_count - 1);
                total_interval_time += report.avg_message_interval * (report.message_count - 1);
            }
        }
        
        if (total_intervals > 0) {
            aggregated.avg_message_interval = std::chrono::milliseconds(
                total_interval_time.count() / total_intervals);
        }
        
        return aggregated;
    }
    
    /**
     * @brief Extract message prefix (first word or pattern)
     * @param message Full log message
     * @return Message prefix
     */
    std::string extractMessagePrefix(const std::string& message) {
        // Remove thread ID prefix if present
        std::string clean_message = message;
        size_t tid_end = message.find("] ");
        if (tid_end != std::string::npos && message.find("[tid:") == 0) {
            clean_message = message.substr(tid_end + 2);
        }
        
        // Extract first word or pattern
        size_t space_pos = clean_message.find(' ');
        if (space_pos != std::string::npos) {
            return clean_message.substr(0, space_pos);
        }
        return clean_message;
    }
    
    /**
     * @brief Check if prefix indicates a slow operation
     * @param prefix Message prefix to check
     * @return True if prefix typically indicates slow operations
     */
    bool isSlowOperationPrefix(const std::string& prefix) const {
        static const std::vector<std::string> slow_prefixes = {
            "SQL_", "AWS_", "DB_", "CONN_", "QUERY_", "API_", "HTTP_", "UPLOAD_", "DOWNLOAD_"
        };
        
        for (const auto& slow_prefix : slow_prefixes) {
            if (prefix.find(slow_prefix) == 0) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * @brief Print report header
     */
    void printHeader() {
        std::cout << "\n" << std::string(80, '=') << "\n";
        std::cout << "                    MULTI-THREAD LOG ANALYSIS REPORT\n";
        std::cout << std::string(80, '=') << "\n\n";
    }
    
    /**
     * @brief Print overall summary statistics
     * @param report Overall report
     */
    void printOverallSummary(const OverallReport& report) {
        std::cout << "OVERALL SUMMARY:\n";
        std::cout << "  Total Execution Time: " << report.total_execution_time.count() << " ms\n";
        std::cout << "  Total Threads: " << report.total_threads << "\n";
        std::cout << "  Total Log Entries: " << report.total_log_entries << "\n";
        std::cout << "  Avg Entries per Thread: " 
                  << (report.total_threads > 0 ? report.total_log_entries / report.total_threads : 0) 
                  << "\n\n";
    }
    
    /**
     * @brief Print per-thread summaries
     * @param thread_reports Vector of thread reports
     */
    void printThreadSummaries(const std::vector<ThreadReport>& thread_reports) {
        std::cout << "PER-THREAD SUMMARY:\n";
        std::cout << std::left 
                  << std::setw(15) << "Thread ID"
                  << std::setw(10) << "Messages"
                  << std::setw(12) << "Duration(ms)"
                  << std::setw(12) << "Avg Int(ms)"
                  << std::setw(8) << "Errors"
                  << std::setw(8) << "SlowOps"
                  << std::setw(10) << "Outliers"
                  << "Top Prefixes\n";
        std::cout << std::string(95, '-') << "\n";
        
        for (const auto& report : thread_reports) {
            std::string thread_display = report.thread_id;
            if (thread_display.length() > 12) {
                thread_display = thread_display.substr(0, 9) + "...";
            }
            
            std::cout << std::left
                      << std::setw(15) << thread_display
                      << std::setw(10) << report.message_count
                      << std::setw(12) << report.total_duration.count()
                      << std::setw(12) << report.avg_message_interval.count()
                      << std::setw(8) << report.error_count
                      << std::setw(8) << report.slow_operations_count
                      << std::setw(10) << report.outlier_count;
                      
            // Print top prefixes
            for (size_t i = 0; i < report.top_message_prefixes.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << report.top_message_prefixes[i];
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    
    /**
     * @brief Print aggregated statistics
     * @param aggregated Aggregated thread report
     */
    void printAggregatedStats(const ThreadReport& aggregated) {
        std::cout << "AGGREGATED STATISTICS:\n";
        std::cout << "  Total Messages: " << aggregated.message_count << "\n";
        std::cout << "  Total Errors: " << aggregated.error_count << "\n";
        std::cout << "  Error Rate: " << std::fixed << std::setprecision(5) 
                  << aggregated.error_rate << "%\n";
        std::cout << "  Total Slow Operations: " << aggregated.slow_operations_count << "\n";
        std::cout << "  Slowest Operation: " << aggregated.slowest_operation.count() << " ms\n";
        std::cout << "  Total Outliers: " << aggregated.outlier_count << "\n";
        std::cout << "  Average Message Interval: " << aggregated.avg_message_interval.count() << " ms\n\n";
    }
    
    /**
     * @brief Print detailed slow operations table
     * @param slow_operations Vector of slow operations to display
     */
    void printSlowOperationsTable(const std::vector<SlowOperation>& slow_operations) {
        if (slow_operations.empty()) {
            std::cout << "SLOW OPERATIONS ANALYSIS:\n";
            std::cout << "  No slow operations detected (threshold: 100ms)\n\n";
            return;
        }
        
        std::cout << "SLOW OPERATIONS ANALYSIS:\n";
        std::cout << "  Found " << slow_operations.size() << " slow operations (>100ms duration)\n\n";
        
        // Sort by duration (slowest first)
        auto sorted_ops = slow_operations;
        std::sort(sorted_ops.begin(), sorted_ops.end(),
                 [](const SlowOperation& a, const SlowOperation& b) {
                     return a.duration > b.duration;
                 });
        
        // Print compact summary table first
        std::cout << "SUMMARY TABLE:\n";
        std::cout << std::left
                  << std::setw(20) << "Timestamp"
                  << std::setw(8) << "Level"
                  << std::setw(15) << "Thread ID"
                  << std::setw(12) << "Duration(ms)"
                  << std::setw(15) << "Prefix"
                  << "Message (truncated)\n";
        std::cout << std::string(120, '-') << "\n";
        
        // Print each slow operation in compact format
        for (const auto& op : sorted_ops) {
            // Format timestamp
            auto time_t = std::chrono::system_clock::to_time_t(op.timestamp);
            std::tm* tm_info = std::localtime(&time_t);
            char timestamp_str[20];
            std::strftime(timestamp_str, sizeof(timestamp_str), "%H:%M:%S", tm_info);
            
            // Get milliseconds part
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                op.timestamp.time_since_epoch()) % 1000;
            std::string full_timestamp = std::string(timestamp_str) + "." + 
                                       std::to_string(ms.count()).substr(0, 3);
            
            // Format thread ID (truncate if too long)
            std::string thread_display = op.thread_id;
            if (thread_display.length() > 12) {
                thread_display = thread_display.substr(0, 9) + "...";
            }
            
            // Format log level
            std::string level_str;
            switch (op.level) {
                case LogLevel::TRACE: level_str = "TRACE"; break;
                case LogLevel::DEBUG: level_str = "DEBUG"; break;
                case LogLevel::INFO: level_str = "INFO"; break;
                case LogLevel::WARN: level_str = "WARN"; break;
                case LogLevel::ERROR: level_str = "ERROR"; break;
                case LogLevel::FATAL: level_str = "FATAL"; break;
                case LogLevel::OFF: level_str = "OFF"; break;
            }
            
            // Truncate message for summary table
            std::string display_message = op.full_message;
            // Remove thread ID prefix from message if present
            size_t tid_end = display_message.find("] ");
            if (tid_end != std::string::npos && display_message.find("[tid:") == 0) {
                display_message = display_message.substr(tid_end + 2);
            }
            if (display_message.length() > 50) {
                display_message = display_message.substr(0, 47) + "...";
            }
            
            std::cout << std::left
                      << std::setw(20) << full_timestamp
                      << std::setw(8) << level_str
                      << std::setw(15) << thread_display
                      << std::setw(12) << op.duration.count()
                      << std::setw(15) << op.prefix
                      << display_message << "\n";
        }
        
        // Print detailed section with full messages
        std::cout << "\nDETAILED SLOW OPERATIONS (Full Messages):\n";
        std::cout << std::string(100, '=') << "\n";
        
        for (size_t i = 0; i < sorted_ops.size(); ++i) {
            const auto& op = sorted_ops[i];
            
            // Format timestamp
            auto time_t = std::chrono::system_clock::to_time_t(op.timestamp);
            std::tm* tm_info = std::localtime(&time_t);
            char timestamp_str[20];
            std::strftime(timestamp_str, sizeof(timestamp_str), "%H:%M:%S", tm_info);
            
            // Get milliseconds part
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                op.timestamp.time_since_epoch()) % 1000;
            std::string full_timestamp = std::string(timestamp_str) + "." + 
                                       std::to_string(ms.count()).substr(0, 3);
            
            // Format log level
            std::string level_str;
            switch (op.level) {
                case LogLevel::TRACE: level_str = "TRACE"; break;
                case LogLevel::DEBUG: level_str = "DEBUG"; break;
                case LogLevel::INFO: level_str = "INFO"; break;
                case LogLevel::WARN: level_str = "WARN"; break;
                case LogLevel::ERROR: level_str = "ERROR"; break;
                case LogLevel::FATAL: level_str = "FATAL"; break;
                case LogLevel::OFF: level_str = "OFF"; break;
            }
            
            // Remove thread ID prefix from message if present
            std::string clean_message = op.full_message;
            size_t tid_end = clean_message.find("] ");
            if (tid_end != std::string::npos && clean_message.find("[tid:") == 0) {
                clean_message = clean_message.substr(tid_end + 2);
            }
            
            std::cout << "[" << (i + 1) << "] Duration: " << op.duration.count() << "ms | "
                      << "Time: " << full_timestamp << " | "
                      << "Level: " << level_str << " | "
                      << "Thread: " << op.thread_id << "\n";
            std::cout << "    Prefix: " << op.prefix << "\n";
            std::cout << "    Full Message: " << clean_message << "\n";
            
            if (i < sorted_ops.size() - 1) {
                std::cout << std::string(100, '-') << "\n";
            }
        }
        std::cout << std::string(100, '=') << "\n\n";
    }
    
    /**
     * @brief Print report footer
     */
    void printFooter() {
        std::cout << std::string(80, '=') << "\n";
        std::cout << "Report generated by ulog Multi-Thread Analyzer\n";
        std::cout << std::string(80, '=') << "\n\n";
    }
};

} // namespace extensions
} // namespace ulog

#endif // ULOG_MULTI_THREAD_REPORTER_H
