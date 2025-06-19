#ifndef ULOG_BUFFER_STATS_REPORTER_H
#define ULOG_BUFFER_STATS_REPORTER_H

/**
 * @file buffer_stats_reporter.h
 * @brief Reporting utilities for BufferStats analysis
 * @author ulog team
 * @version 1.0.0
 * 
 * This header provides reporting and analysis utilities for BufferStats,
 * generating human-readable reports with insights and recommendations
 * based on log buffer statistical analysis.
 */

#include "buffer_stats.h"
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

namespace ulog {
namespace extensions {

/**
 * @brief Reporter class for generating human-readable BufferStats reports
 * 
 * This class takes BufferStats analysis results and generates comprehensive
 * reports with insights, recommendations, and formatted statistics. It can
 * output reports to console, strings, or files for further analysis.
 * 
 * @code{.cpp}
 * BufferStats stats(logger);
 * BufferStatsReporter reporter(stats);
 * 
 * std::cout << reporter.generate_full_report() << std::endl;
 * reporter.save_report("analysis_report.txt");
 * @endcode
 */
class BufferStatsReporter {
public:
    /**
     * @brief Constructor
     * @param stats Reference to BufferStats instance to report on
     */
    explicit BufferStatsReporter(const BufferStats& stats) : stats_(stats) {}
    
    /**
     * @brief Generate a summary report with key metrics
     * @return String containing formatted summary report
     */
    std::string generate_summary_report() const {
        std::ostringstream oss;
        oss << "=== Buffer Statistics Summary ===" << std::endl;
        oss << std::endl;
        
        // Basic metrics
        auto total = stats_.total_messages();
        oss << "Total Messages: " << total << std::endl;
        
        if (total == 0) {
            oss << "No messages in buffer to analyze." << std::endl;
            return oss.str();
        }
        
        // Level distribution
        auto distribution = stats_.level_distribution();
        oss << "Message Distribution by Level:" << std::endl;
        for (const auto& pair : distribution) {
            double percentage = (static_cast<double>(pair.second) / static_cast<double>(total)) * 100.0;
            oss << "  " << to_string(pair.first) << ": " << pair.second 
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
        }
        oss << std::endl;
        
        // Timing metrics
        auto timespan = stats_.total_timespan();
        auto avg_interval = stats_.average_interval();
        auto median_interval = stats_.median_interval();
        
        oss << "Timing Analysis:" << std::endl;
        oss << "  Total Timespan: " << timespan.count() << " ms" << std::endl;
        oss << "  Average Interval: " << avg_interval.count() << " ms" << std::endl;
        oss << "  Median Interval: " << median_interval.count() << " ms" << std::endl;
        oss << std::endl;
        
        // Top prefixes
        auto top_prefixes = stats_.top_prefixes(5, 15);
        if (!top_prefixes.empty()) {
            oss << "Top 5 Message Prefixes:" << std::endl;
            for (const auto& pair : top_prefixes) {
                double percentage = (static_cast<double>(pair.second) / static_cast<double>(total)) * 100.0;
                oss << "  \"" << pair.first << "\": " << pair.second
                    << " (" << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
            }
            oss << std::endl;
        }
        
        return oss.str();
    }
    
    /**
     * @brief Generate a performance-focused report
     * @return String containing formatted performance report
     */
    std::string generate_performance_report() const {
        std::ostringstream oss;
        oss << "=== Performance Analysis Report ===" << std::endl;
        oss << std::endl;
        
        // Define common slow operation prefixes
        std::vector<std::string> slow_prefixes = {
            "SQL_", "AWS_", "DB_", "HTTP_", "NETWORK_", "IO_", "DISK_"
        };
        
        auto perf_metrics = stats_.analyze_slow_operations(slow_prefixes);
        
        if (perf_metrics.slow_operations_count > 0) {
            oss << "Slow Operations Analysis:" << std::endl;
            oss << "  Total Slow Operations: " << perf_metrics.slow_operations_count << std::endl;
            oss << "  Slowest Operation: " << perf_metrics.slowest_operation.count() << " ms" << std::endl;
            oss << "  Average Slow Operation: " << perf_metrics.average_slow_operation.count() << " ms" << std::endl;
            oss << std::endl;
            
            if (!perf_metrics.slow_operations.empty()) {
                oss << "Top 10 Slowest Operations:" << std::endl;
                size_t count = 0;
                for (const auto& op : perf_metrics.slow_operations) {
                    if (count >= 10) break;
                    oss << "  " << std::setw(6) << op.second.count() << " ms - " << op.first << std::endl;
                    count++;
                }
                oss << std::endl;
            }
            
            // Performance insights
            oss << "Performance Insights:" << std::endl;
            
            if (perf_metrics.slowest_operation.count() > 1000) {
                oss << "  ⚠ Very slow operations detected (>1s). Consider optimization." << std::endl;
            }
            
            auto total_messages = stats_.total_messages();
            if (total_messages > 0) {
                double slow_percentage = (static_cast<double>(perf_metrics.slow_operations_count) / static_cast<double>(total_messages)) * 100.0;
                if (slow_percentage > 20.0) {
                    oss << "  ⚠ High percentage of slow operations (" << std::fixed << std::setprecision(1) 
                        << slow_percentage << "%). Review system performance." << std::endl;
                }
            }
            
            if (perf_metrics.average_slow_operation.count() > 500) {
                oss << "  ⚠ Average slow operation time is high. Check for consistent bottlenecks." << std::endl;
            }
        } else {
            oss << "No slow operations detected with standard prefixes." << std::endl;
        }
        oss << std::endl;
        
        // Contention analysis
        std::vector<std::string> contention_prefixes = {
            "LOCK_", "MUTEX_", "SEMAPHORE_", "CONDITION_", "WAIT_", "SYNC_"
        };
        
        auto contention_metrics = stats_.analyze_contention(contention_prefixes);
        
        if (contention_metrics.variable_delay_count > 0) {
            oss << "Lock Contention Analysis:" << std::endl;
            oss << "  Operations with Variable Delays: " << contention_metrics.variable_delay_count << std::endl;
            oss << "  Max Delay: " << contention_metrics.max_delay.count() << " ms" << std::endl;
            oss << "  Min Delay: " << contention_metrics.min_delay.count() << " ms" << std::endl;
            oss << "  Average Delay: " << contention_metrics.average_delay.count() << " ms" << std::endl;
            oss << std::endl;
            
            // Contention patterns
            if (!contention_metrics.contention_patterns.empty()) {
                oss << "Contention Patterns by Prefix:" << std::endl;
                for (const auto& pattern : contention_metrics.contention_patterns) {
                    if (!pattern.second.empty()) {
                        auto sum = std::accumulate(pattern.second.begin(), pattern.second.end(), 
                                                 std::chrono::milliseconds(0));
                        auto avg = sum / pattern.second.size();
                        auto max_it = std::max_element(pattern.second.begin(), pattern.second.end());
                        auto min_it = std::min_element(pattern.second.begin(), pattern.second.end());
                        
                        oss << "  " << pattern.first << ": " << pattern.second.size() << " ops, "
                            << "avg=" << avg.count() << "ms, "
                            << "range=" << min_it->count() << "-" << max_it->count() << "ms" << std::endl;
                    }
                }
                oss << std::endl;
            }
            
            // Contention insights
            oss << "Contention Insights:" << std::endl;
            auto delay_range = contention_metrics.max_delay.count() - contention_metrics.min_delay.count();
            if (delay_range > contention_metrics.average_delay.count() * 3) {
                oss << "  ⚠ High variability in contention delays detected. Check for lock contention." << std::endl;
            }
            
            if (contention_metrics.max_delay.count() > 2000) {
                oss << "  ⚠ Very long contention delays detected (>2s). Review locking strategy." << std::endl;
            }
        } else {
            oss << "No lock contention patterns detected." << std::endl;
        }
        
        return oss.str();
    }
    
    /**
     * @brief Generate an outlier analysis report
     * @return String containing formatted outlier report
     */
    std::string generate_outlier_report() const {
        std::ostringstream oss;
        oss << "=== Outlier Analysis Report ===" << std::endl;
        oss << std::endl;
        
        // 90th percentile outliers
        auto outliers_90 = stats_.delay_outliers(90.0);
        if (!outliers_90.empty()) {
            oss << "90th Percentile Delay Outliers (" << outliers_90.size() << " found):" << std::endl;
            size_t count = 0;
            for (const auto& outlier : outliers_90) {
                if (count >= 10) break; // Show top 10
                oss << "  #" << outlier.message_index << " - " << outlier.delay_after.count() 
                    << "ms (threshold: " << outlier.percentile_threshold.count() << "ms)" << std::endl;
                oss << "    \"" << outlier.message_prefix << "\"" << std::endl;
                count++;
            }
            oss << std::endl;
        }
        
        // 95th percentile outliers
        auto outliers_95 = stats_.delay_outliers(95.0);
        if (!outliers_95.empty()) {
            oss << "95th Percentile Delay Outliers (" << outliers_95.size() << " found):" << std::endl;
            size_t count = 0;
            for (const auto& outlier : outliers_95) {
                if (count >= 5) break; // Show top 5
                oss << "  #" << outlier.message_index << " - " << outlier.delay_after.count() 
                    << "ms (threshold: " << outlier.percentile_threshold.count() << "ms)" << std::endl;
                oss << "    \"" << outlier.message_prefix << "\"" << std::endl;
                count++;
            }
            oss << std::endl;
        }
        
        // Outlier insights
        if (!outliers_90.empty() || !outliers_95.empty()) {
            oss << "Outlier Insights:" << std::endl;
            
            if (static_cast<double>(outliers_95.size()) > static_cast<double>(outliers_90.size()) * 0.5) {
                oss << "  ⚠ High concentration of extreme outliers. Check for systematic issues." << std::endl;
            }
            
            if (!outliers_90.empty() && outliers_90[0].delay_after.count() > 5000) {
                oss << "  ⚠ Very extreme outliers detected (>5s). Investigate specific operations." << std::endl;
            }
            
            // Analyze outlier patterns
            std::unordered_map<std::string, size_t> outlier_prefixes;
            for (const auto& outlier : outliers_90) {
                std::string key = outlier.message_prefix.substr(0, 10); // First 10 chars
                outlier_prefixes[key]++;
            }
            
            if (!outlier_prefixes.empty()) {
                auto max_prefix = std::max_element(outlier_prefixes.begin(), outlier_prefixes.end(),
                    [](const auto& a, const auto& b) { return a.second < b.second; });
                
                if (static_cast<double>(max_prefix->second) > static_cast<double>(outliers_90.size()) * 0.3) {
                    oss << "  ⚠ Outliers concentrated in operations starting with \"" 
                        << max_prefix->first << "\" (" << max_prefix->second << " occurrences)." << std::endl;
                }
            }
        } else {
            oss << "No significant delay outliers detected." << std::endl;
        }
        
        return oss.str();
    }
    
    /**
     * @brief Generate a comprehensive full report
     * @return String containing complete analysis report
     */
    std::string generate_full_report() const {
        std::ostringstream oss;
        oss << "=== COMPREHENSIVE BUFFER ANALYSIS REPORT ===" << std::endl;
        oss << "Generated: " << get_current_timestamp() << std::endl;
        oss << std::endl;
        
        oss << generate_summary_report() << std::endl;
        oss << generate_performance_report() << std::endl;
        oss << generate_outlier_report() << std::endl;
        
        // Additional insights and recommendations
        oss << "=== Overall Recommendations ===" << std::endl;
        
        auto total = stats_.total_messages();
        if (total == 0) {
            oss << "No data available for analysis." << std::endl;
            return oss.str();
        }
        
        auto avg_interval = stats_.average_interval();
        auto median_interval = stats_.median_interval();
        
        if (avg_interval.count() > median_interval.count() * 2) {
            oss << "• Investigate operations causing irregular timing patterns." << std::endl;
        }
        
        auto distribution = stats_.level_distribution();
        auto error_count = distribution[LogLevel::ERROR] + distribution[LogLevel::FATAL];
        if (static_cast<double>(error_count) > static_cast<double>(total) * 0.1) {
            oss << "• High error rate detected (" << error_count << "/" << total 
                << "). Review error handling and system stability." << std::endl;
        }
        
        auto outliers = stats_.delay_outliers(95.0);
        if (static_cast<double>(outliers.size()) > static_cast<double>(total) * 0.05) {
            oss << "• Frequent performance outliers detected. Consider performance optimization." << std::endl;
        }
        
        if (total > 100 && avg_interval.count() < 10) {
            oss << "• Very high logging frequency detected. Consider log level filtering or rate limiting." << std::endl;
        }
        
        oss << std::endl;
        oss << "=== END OF REPORT ===" << std::endl;
        
        return oss.str();
    }
    
    /**
     * @brief Print report to output stream
     * @param os Output stream to print to (default: std::cout)
     */
    void print_report(std::ostream& os = std::cout) const {
        os << generate_full_report();
    }
    
    /**
     * @brief Save report to file
     * @param filename Path to file where report should be saved
     * @throws std::runtime_error if file cannot be opened for writing
     */
    void save_report(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file for writing: " + filename);
        }
        
        file << generate_full_report();
        file.close();
    }

private:
    const BufferStats& stats_;
    
    /**
     * @brief Get current timestamp as string
     * @return Formatted timestamp string
     */
    std::string get_current_timestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));
        
        return std::string(time_str);
    }
};

} // namespace extensions
} // namespace ulog

#endif // ULOG_BUFFER_STATS_REPORTER_H
