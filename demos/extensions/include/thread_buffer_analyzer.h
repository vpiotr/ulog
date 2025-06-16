#ifndef ULOG_THREAD_BUFFER_ANALYZER_H
#define ULOG_THREAD_BUFFER_ANALYZER_H

/**
 * @file thread_buffer_analyzer.h
 * @brief Multi-thread buffer analysis for organizing log entries by thread
 * @author ulog team
 * @version 1.0.0
 * 
 * This extension provides functionality to analyze log buffer contents
 * and organize entries by thread ID extracted from message prefixes.
 */

#include "ulog/ulog.h"
#include <unordered_map>
#include <vector>
#include <regex>
#include <functional>
#include <algorithm>
#include <numeric>

namespace ulog {
namespace extensions {

/**
 * @brief Thread distribution statistics
 */
struct ThreadDistributionStats {
    size_t total_threads;           ///< Number of unique threads
    size_t total_entries;           ///< Total number of log entries
    size_t max_entries_per_thread;  ///< Maximum entries for any thread
    size_t min_entries_per_thread;  ///< Minimum entries for any thread
    double avg_entries_per_thread;  ///< Average entries per thread
    std::string most_active_thread; ///< Thread ID with most entries
    std::string least_active_thread;///< Thread ID with least entries
};

/**
 * @brief Analyzes buffer contents by thread ID, organizing entries per thread
 * 
 * This class parses log entries to extract thread IDs from messages formatted
 * with the pattern [tid:<value>] and organizes them into per-thread collections.
 * Supports filtering via predicates for advanced analysis scenarios.
 * 
 * @code{.cpp}
 * ThreadBufferAnalyzer analyzer;
 * auto thread_entries = analyzer.analyzeByThread(logger.buffer());
 * 
 * // Analyze with filter (only ERROR level messages)
 * auto error_predicate = [](const LogEntry& entry) {
 *     return entry.level == LogLevel::ERROR;
 * };
 * auto error_by_thread = analyzer.analyzeByThread(logger.buffer(), error_predicate);
 * @endcode
 */
class ThreadBufferAnalyzer {
public:
    using ThreadId = std::string;
    using LogEntryPredicate = std::function<bool(const LogEntry&)>;
    using ThreadEntryMap = std::unordered_map<ThreadId, std::vector<LogEntry>>;
    
private:
    // Regex pattern for thread ID extraction: [tid:<value>]
    static const std::regex THREAD_ID_PATTERN;
    static const std::string UNKNOWN_THREAD_ID;
    
public:
    /**
     * @brief Analyze buffer and organize entries by thread ID
     * @param buffer Buffer to analyze
     * @param predicate Optional filter predicate (default: accept all)
     * @return Map of thread ID to vector of log entries
     */
    ThreadEntryMap analyzeByThread(const ulog::LogBuffer& buffer, 
                                  LogEntryPredicate predicate = nullptr) const {
        ThreadEntryMap result;
        
        for (auto it = buffer.cbegin(); it != buffer.cend(); ++it) {
            if (!predicate || predicate(*it)) {
                std::string thread_id = extractThreadId(it->message);
                result[thread_id].push_back(*it);
            }
        }
        
        return result;
    }
    
    /**
     * @brief Analyze iterable collection and organize entries by thread ID
     * @tparam Iterator Iterator type for log entries
     * @param begin Iterator to first entry
     * @param end Iterator past last entry
     * @param predicate Optional filter predicate (default: accept all)
     * @return Map of thread ID to vector of log entries
     */
    template<typename Iterator>
    ThreadEntryMap analyzeByThread(Iterator begin, Iterator end,
                                  LogEntryPredicate predicate = nullptr) const {
        ThreadEntryMap result;
        
        for (auto it = begin; it != end; ++it) {
            if (!predicate || predicate(*it)) {
                std::string thread_id = extractThreadId(it->message);
                result[thread_id].push_back(*it);
            }
        }
        
        return result;
    }
    
    /**
     * @brief Extract thread ID from log message
     * @param message Log message to parse
     * @return Thread ID if found, "unknown" otherwise
     */
    std::string extractThreadId(const std::string& message) const {
        std::smatch match;
        if (std::regex_search(message, match, THREAD_ID_PATTERN)) {
            return match[1].str(); // Return captured group (thread ID)
        }
        return UNKNOWN_THREAD_ID;
    }
    
    /**
     * @brief Check if message contains thread ID pattern
     * @param message Message to check
     * @return True if thread ID pattern found
     */
    bool hasThreadId(const std::string& message) const {
        return std::regex_search(message, THREAD_ID_PATTERN);
    }
    
    /**
     * @brief Get list of all unique thread IDs from buffer
     * @param buffer Buffer to analyze
     * @return Vector of unique thread IDs
     */
    std::vector<ThreadId> getThreadIds(const ulog::LogBuffer& buffer) const {
        std::unordered_map<ThreadId, bool> unique_ids;
        
        for (auto it = buffer.cbegin(); it != buffer.cend(); ++it) {
            std::string thread_id = extractThreadId(it->message);
            unique_ids[thread_id] = true;
        }
        
        std::vector<ThreadId> result;
        result.reserve(unique_ids.size());
        for (const auto& pair : unique_ids) {
            result.push_back(pair.first);
        }
        
        return result;
    }
    
    /**
     * @brief Get statistics about thread distribution
     * @param thread_entries Map of thread entries
     * @return Summary statistics
     */
    ThreadDistributionStats getDistributionStats(const ThreadEntryMap& thread_entries) const {
        ThreadDistributionStats stats{};
        
        if (thread_entries.empty()) {
            return stats;
        }
        
        stats.total_threads = thread_entries.size();
        stats.total_entries = 0;
        stats.max_entries_per_thread = 0;
        stats.min_entries_per_thread = SIZE_MAX;
        
        for (const auto& pair : thread_entries) {
            size_t entry_count = pair.second.size();
            stats.total_entries += entry_count;
            
            if (entry_count > stats.max_entries_per_thread) {
                stats.max_entries_per_thread = entry_count;
                stats.most_active_thread = pair.first;
            }
            
            if (entry_count < stats.min_entries_per_thread) {
                stats.min_entries_per_thread = entry_count;
                stats.least_active_thread = pair.first;
            }
        }
        
        stats.avg_entries_per_thread = static_cast<double>(stats.total_entries) / stats.total_threads;
        
        return stats;
    }
};

// Static member definitions
const std::regex ThreadBufferAnalyzer::THREAD_ID_PATTERN{R"(\[tid:([^\]]+)\])"};
const std::string ThreadBufferAnalyzer::UNKNOWN_THREAD_ID{"unknown"};

} // namespace extensions
} // namespace ulog

#endif // ULOG_THREAD_BUFFER_ANALYZER_H
