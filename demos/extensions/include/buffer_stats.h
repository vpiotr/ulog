#ifndef ULOG_BUFFER_STATS_H
#define ULOG_BUFFER_STATS_H

/**
 * @file buffer_stats.h
 * @brief Buffer statistics extensions for ulog performance analysis
 * @author ulog team
 * @version 1.0.0
 * 
 * This extension provides comprehensive statistical analysis capabilities for log
 * buffer contents, focusing on performance metrics, message pattern analysis,
 * and outlier detection. Unlike BufferAssertions which validates conditions,
 * BufferStats calculates and reports actual metrics without validation.
 */

#include "ulog/ulog.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <functional>
#include <chrono>
#include <string>
#include <utility>
#include <cmath>

namespace ulog {
namespace extensions {

/**
 * @brief Structure representing a delay outlier in log messages
 */
struct DelayOutlier {
    size_t message_index;                           ///< Index of the message in buffer
    std::string message_prefix;                     ///< First N characters of the message
    std::chrono::milliseconds delay_after;          ///< Delay after this message
    std::chrono::milliseconds percentile_threshold; ///< The percentile threshold exceeded
    
    DelayOutlier(size_t idx, const std::string& prefix, 
                std::chrono::milliseconds delay, std::chrono::milliseconds threshold)
        : message_index(idx), message_prefix(prefix), delay_after(delay), percentile_threshold(threshold) {}
};

/**
 * @brief Performance metrics for slow operations analysis
 */
struct PerformanceMetrics {
    size_t slow_operations_count;                                                    ///< Number of slow operations found
    std::chrono::milliseconds slowest_operation;                                     ///< Duration of the slowest operation
    std::chrono::milliseconds average_slow_operation;                               ///< Average duration of slow operations
    std::vector<std::pair<std::string, std::chrono::milliseconds>> slow_operations; ///< List of slow operations with delays
};

/**
 * @brief Lock contention analysis metrics
 */
struct ContentionMetrics {
    size_t variable_delay_count;                                                                          ///< Number of operations with variable delays
    std::chrono::milliseconds max_delay;                                                                  ///< Maximum delay observed
    std::chrono::milliseconds min_delay;                                                                  ///< Minimum delay observed
    std::chrono::milliseconds average_delay;                                                              ///< Average delay
    std::vector<std::pair<std::string, std::vector<std::chrono::milliseconds>>> contention_patterns;     ///< Contention patterns by prefix
};

/**
 * @brief Buffer statistics utility class for analyzing log buffer contents
 * 
 * This class provides comprehensive statistical analysis of log buffer contents,
 * including timing analysis, message pattern detection, outlier identification,
 * and performance metrics calculation. It's designed to help identify performance
 * bottlenecks and patterns in application logging.
 * 
 * @code{.cpp}
 * auto& logger = ulog::getLogger("app");
 * logger.enable_buffer(1000);
 * 
 * // Generate some log messages
 * logger.info("SQL_SELECT: user query");
 * std::this_thread::sleep_for(std::chrono::milliseconds(200));
 * logger.info("CACHE_HIT: user data found");
 * 
 * // Analyze buffer statistics
 * BufferStats stats(logger);
 * auto outliers = stats.delay_outliers(90.0);
 * auto slow_ops = stats.analyze_slow_operations({"SQL_", "AWS_"});
 * @endcode
 */
class BufferStats {
public:
    /**
     * @brief Constructor
     * @param logger Reference to the logger to analyze
     * @throws std::runtime_error if logger doesn't have buffer enabled
     */
    explicit BufferStats(ulog::Logger& logger) : logger_(logger) {
        if (!logger_.buffer()) {
            throw std::runtime_error("Logger must have buffer enabled for statistics analysis");
        }
    }
    
    // Basic Statistics
    
    /**
     * @brief Get total number of messages in buffer
     * @return Total message count
     */
    size_t total_messages() const {
        auto buffer = logger_.buffer();
        return buffer->size();
    }
    
    /**
     * @brief Count messages by specific log level
     * @param level Log level to count
     * @return Number of messages with specified level
     */
    size_t messages_by_level(LogLevel level) const {
        auto buffer = logger_.buffer();
        size_t count = 0;
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            if (it->level == level) {
                count++;
            }
        }
        return count;
    }
    
    /**
     * @brief Get distribution of messages by log level
     * @return Map of log levels to message counts
     */
    std::map<LogLevel, size_t> level_distribution() const {
        auto buffer = logger_.buffer();
        std::map<LogLevel, size_t> distribution;
        
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            distribution[it->level]++;
        }
        
        return distribution;
    }
    
    // Timing Statistics
    
    /**
     * @brief Calculate total timespan covered by buffer messages
     * @return Duration from first to last message, or zero if buffer is empty or has only one message
     */
    std::chrono::milliseconds total_timespan() const {
        auto buffer = logger_.buffer();
        if (buffer->size() < 2) {
            return std::chrono::milliseconds(0);
        }
        
        auto first = buffer->cbegin();
        auto last = buffer->cend() - 1;
        
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            last->timestamp - first->timestamp
        );
    }
    
    /**
     * @brief Calculate average time interval between consecutive messages
     * @return Average interval, or zero if buffer has less than 2 messages
     */
    std::chrono::milliseconds average_interval() const {
        auto intervals = all_intervals();
        if (intervals.empty()) {
            return std::chrono::milliseconds(0);
        }
        
        auto sum = std::accumulate(intervals.begin(), intervals.end(), std::chrono::milliseconds(0));
        return sum / intervals.size();
    }
    
    /**
     * @brief Calculate median time interval between consecutive messages
     * @return Median interval, or zero if buffer has less than 2 messages
     */
    std::chrono::milliseconds median_interval() const {
        auto intervals = all_intervals();
        if (intervals.empty()) {
            return std::chrono::milliseconds(0);
        }
        
        std::sort(intervals.begin(), intervals.end());
        size_t middle = intervals.size() / 2;
        
        if (intervals.size() % 2 == 0) {
            return (intervals[middle - 1] + intervals[middle]) / 2;
        } else {
            return intervals[middle];
        }
    }
    
    /**
     * @brief Get all time intervals between consecutive messages
     * @return Vector of intervals in chronological order
     */
    std::vector<std::chrono::milliseconds> all_intervals() const {
        auto buffer = logger_.buffer();
        std::vector<std::chrono::milliseconds> intervals;
        
        if (buffer->size() < 2) {
            return intervals;
        }
        
        auto prev_it = buffer->cbegin();
        for (auto it = buffer->cbegin() + 1; it != buffer->cend(); ++it) {
            auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(
                it->timestamp - prev_it->timestamp
            );
            intervals.push_back(interval);
            prev_it = it;
        }
        
        return intervals;
    }
    
    // Message Pattern Analysis
    
    /**
     * @brief Get top N most frequent message prefixes
     * @param n Number of top prefixes to return
     * @param prefix_length Length of prefix to consider
     * @return Vector of (prefix, count) pairs sorted by count descending
     */
    std::vector<std::pair<std::string, size_t>> top_prefixes(size_t n = 10, size_t prefix_length = 20) const {
        auto buffer = logger_.buffer();
        std::unordered_map<std::string, size_t> prefix_counts;
        
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            std::string prefix = it->message.substr(0, std::min(prefix_length, it->message.length()));
            prefix_counts[prefix]++;
        }
        
        std::vector<std::pair<std::string, size_t>> result(prefix_counts.begin(), prefix_counts.end());
        std::sort(result.begin(), result.end(), 
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        if (result.size() > n) {
            result.resize(n);
        }
        
        return result;
    }
    
    /**
     * @brief Get top N most frequent complete messages
     * @param n Number of top messages to return
     * @return Vector of (message, count) pairs sorted by count descending
     */
    std::vector<std::pair<std::string, size_t>> message_frequency(size_t n = 10) const {
        auto buffer = logger_.buffer();
        std::unordered_map<std::string, size_t> message_counts;
        
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            message_counts[it->message]++;
        }
        
        std::vector<std::pair<std::string, size_t>> result(message_counts.begin(), message_counts.end());
        std::sort(result.begin(), result.end(), 
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        if (result.size() > n) {
            result.resize(n);
        }
        
        return result;
    }
    
    // Outlier Detection
    
    /**
     * @brief Find delay outliers - messages followed by unusually long delays
     * @param percentile Percentile threshold (e.g., 90.0 for 90th percentile)
     * @return Vector of delay outliers sorted by delay descending
     */
    std::vector<DelayOutlier> delay_outliers(double percentile = 90.0) const {
        auto intervals = all_intervals();
        if (intervals.empty()) {
            return {};
        }
        
        // Calculate percentile threshold
        std::vector<std::chrono::milliseconds> sorted_intervals = intervals;
        std::sort(sorted_intervals.begin(), sorted_intervals.end());
        
        size_t percentile_index = static_cast<size_t>((percentile / 100.0) * static_cast<double>(sorted_intervals.size()));
        if (percentile_index >= sorted_intervals.size()) {
            percentile_index = sorted_intervals.size() - 1;
        }
        auto threshold = sorted_intervals[percentile_index];
        
        // Find outliers
        std::vector<DelayOutlier> outliers;
        auto buffer = logger_.buffer();
        auto interval_it = intervals.begin();
        
        size_t index = 0;
        for (auto it = buffer->cbegin(); it != buffer->cend() - 1; ++it, ++interval_it, ++index) {
            if (*interval_it > threshold) {
                std::string prefix = it->message.substr(0, std::min(static_cast<size_t>(30), it->message.length()));
                outliers.emplace_back(index, prefix, *interval_it, threshold);
            }
        }
        
        // Sort by delay descending
        std::sort(outliers.begin(), outliers.end(), 
                  [](const auto& a, const auto& b) { return a.delay_after > b.delay_after; });
        
        return outliers;
    }
    
    // Advanced Analysis with Lambdas
    
    /**
     * @brief Filter messages using custom predicate
     * @tparam Predicate Function type taking const LogEntry& and returning bool
     * @param pred Predicate function to filter messages
     * @return Vector of matching log entries
     */
    template<typename Predicate>
    std::vector<LogEntry> filter_messages(Predicate pred) const {
        auto buffer = logger_.buffer();
        std::vector<LogEntry> result;
        
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            if (pred(*it)) {
                result.push_back(*it);
            }
        }
        
        return result;
    }
    
    /**
     * @brief Get top N messages by custom criteria
     * @tparam Extractor Function type extracting comparison value from LogEntry
     * @tparam Comparator Function type comparing extracted values
     * @param n Number of top messages to return
     * @param extract Function to extract comparison value from LogEntry
     * @param comp Comparator function (default: greater than for descending order)
     * @return Vector of top N log entries
     */
    template<typename Extractor, typename Comparator = std::greater<>>
    std::vector<LogEntry> top_messages_by(size_t n, Extractor extract, Comparator comp = Comparator{}) const {
        auto buffer = logger_.buffer();
        std::vector<LogEntry> entries(buffer->cbegin(), buffer->cend());
        
        std::sort(entries.begin(), entries.end(), 
                  [&extract, &comp](const auto& a, const auto& b) { 
                      return comp(extract(a), extract(b)); 
                  });
        
        if (entries.size() > n) {
            entries.erase(entries.begin() + static_cast<std::ptrdiff_t>(n), entries.end());
        }
        
        return entries;
    }
    
    // Performance Metrics
    
    /**
     * @brief Analyze slow operations based on message prefixes
     * @param slow_prefixes Vector of prefixes that identify slow operations
     * @return Performance metrics for identified slow operations
     */
    PerformanceMetrics analyze_slow_operations(const std::vector<std::string>& slow_prefixes) const {
        auto buffer = logger_.buffer();
        auto intervals = all_intervals();
        
        PerformanceMetrics metrics{};
        std::vector<std::chrono::milliseconds> slow_durations;
        
        if (intervals.empty()) {
            return metrics;
        }
        
        auto interval_it = intervals.begin();
        for (auto it = buffer->cbegin(); it != buffer->cend() - 1; ++it, ++interval_it) {
            // Check if message starts with any slow prefix
            bool is_slow = std::any_of(slow_prefixes.begin(), slow_prefixes.end(),
                [&it](const std::string& prefix) {
                    return it->message.find(prefix) == 0;
                });
            
            if (is_slow) {
                metrics.slow_operations_count++;
                slow_durations.push_back(*interval_it);
                
                std::string prefix = it->message.substr(0, std::min(static_cast<size_t>(50), it->message.length()));
                metrics.slow_operations.emplace_back(prefix, *interval_it);
                
                if (*interval_it > metrics.slowest_operation) {
                    metrics.slowest_operation = *interval_it;
                }
            }
        }
        
        if (!slow_durations.empty()) {
            auto sum = std::accumulate(slow_durations.begin(), slow_durations.end(), std::chrono::milliseconds(0));
            metrics.average_slow_operation = sum / slow_durations.size();
            
            // Sort slow operations by duration descending
            std::sort(metrics.slow_operations.begin(), metrics.slow_operations.end(),
                      [](const auto& a, const auto& b) { return a.second > b.second; });
        }
        
        return metrics;
    }
    
    /**
     * @brief Analyze lock contention patterns based on message prefixes
     * @param contention_prefixes Vector of prefixes that identify contention-prone operations
     * @return Contention metrics for identified operations
     */
    ContentionMetrics analyze_contention(const std::vector<std::string>& contention_prefixes) const {
        auto buffer = logger_.buffer();
        auto intervals = all_intervals();
        
        ContentionMetrics metrics{};
        std::unordered_map<std::string, std::vector<std::chrono::milliseconds>> prefix_delays;
        
        if (intervals.empty()) {
            return metrics;
        }
        
        auto interval_it = intervals.begin();
        for (auto it = buffer->cbegin(); it != buffer->cend() - 1; ++it, ++interval_it) {
            // Check if message starts with any contention prefix
            for (const auto& prefix : contention_prefixes) {
                if (it->message.find(prefix) == 0) {
                    prefix_delays[prefix].push_back(*interval_it);
                    metrics.variable_delay_count++;
                    
                    if (*interval_it > metrics.max_delay) {
                        metrics.max_delay = *interval_it;
                    }
                    if (metrics.min_delay == std::chrono::milliseconds(0) || *interval_it < metrics.min_delay) {
                        metrics.min_delay = *interval_it;
                    }
                    break;
                }
            }
        }
        
        // Calculate average delay and build contention patterns
        std::vector<std::chrono::milliseconds> all_contention_delays;
        for (const auto& pair : prefix_delays) {
            metrics.contention_patterns.push_back(pair);
            all_contention_delays.insert(all_contention_delays.end(), 
                                       pair.second.begin(), pair.second.end());
        }
        
        if (!all_contention_delays.empty()) {
            auto sum = std::accumulate(all_contention_delays.begin(), all_contention_delays.end(), 
                                     std::chrono::milliseconds(0));
            metrics.average_delay = sum / all_contention_delays.size();
        }
        
        return metrics;
    }

private:
    ulog::Logger& logger_;
};

} // namespace extensions
} // namespace ulog

#endif // ULOG_BUFFER_STATS_H
