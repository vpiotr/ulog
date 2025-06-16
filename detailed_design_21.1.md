# Detailed Design for Todo Items 21.1.1 - 21.1.4: Multi-threaded BufferStats Extensions

## Overview

This design document outlines the implementation of multi-threaded BufferStats extensions that allow for per-thread analysis of log messages. The solution maintains the existing architecture without modifying `ulog.h` and follows the established patterns for extensions.

## Design Principles

1. **No modifications to ulog.h**: All new functionality implemented as extensions
2. **Thread-safe by design**: All components handle concurrent access properly
3. **Iterable-based analysis**: BufferStats enhanced to work with any iterable of LogEntry
4. **Reusable components**: Extensions can be used independently
5. **Performance-conscious**: Minimal overhead for thread ID handling

## 21.1.1: Thread ID Format Specification

### Format Definition
```
[tid:<thread_id>] <original_message>
```

### Examples
```
[tid:140234567891] Starting database connection
[tid:140234567892] Processing user request for ID: 12345
[tid:140234567891] SQL_SELECT: SELECT * FROM users WHERE id = ?
```

### Technical Details
- Thread ID extracted using `std::this_thread::get_id()`
- Converted to string representation using `std::ostringstream`
- Format chosen for easy parsing with regex patterns
- Maintains compatibility with existing log analysis tools

## 21.1.2: ThreadIdObserver - Thread-Safe Wrapper

### Class Design

```cpp
namespace ulog {
namespace extensions {

/**
 * @brief Thread-safe observer wrapper that adds thread ID prefix to messages
 * 
 * This class wraps around another observer and automatically prefixes all
 * log messages with the thread ID in the format [tid:<value>] before
 * forwarding them to the target observer.
 */
class ThreadIdObserver : public ulog::LogObserver {
private:
    std::unique_ptr<ulog::LogObserver> target_observer_;
    mutable std::mutex mutex_;
    
public:
    /**
     * @brief Constructor
     * @param target Observer to forward messages to after adding thread prefix
     */
    explicit ThreadIdObserver(std::unique_ptr<ulog::LogObserver> target);
    
    // LogObserver interface implementation
    void handleRegistered(const std::string& logger_name) override;
    void handleUnregistered(const std::string& logger_name) override;
    void handleNewMessage(const LogEntry& entry) override;
    void handleFlush(const std::string& logger_name) override;
    
private:
    /**
     * @brief Get current thread ID as string
     * @return Thread ID formatted as string
     */
    std::string getCurrentThreadId() const;
    
    /**
     * @brief Add thread ID prefix to message
     * @param original_message Original log message
     * @return Message with thread ID prefix
     */
    std::string addThreadIdPrefix(const std::string& original_message) const;
};

} // namespace extensions
} // namespace ulog
```

### Key Features
- Thread-safe message modification
- Maintains original LogEntry structure (only message content changed)
- Forwards all observer events to target
- Minimal performance overhead
- Can wrap any existing observer

### Usage Example
```cpp
// Create file observer for output
auto file_observer = std::make_unique<FileObserver>("app.log");

// Wrap with thread ID observer
auto thread_observer = std::make_unique<ThreadIdObserver>(std::move(file_observer));

// Register with logger
logger.register_observer(std::move(thread_observer));
```

## 21.1.3: ThreadBufferAnalyzer - Multi-thread Buffer Analysis

### Class Design

```cpp
namespace ulog {
namespace extensions {

/**
 * @brief Analyzes buffer contents by thread ID, organizing entries per thread
 * 
 * This class parses log entries to extract thread IDs from messages formatted
 * with the pattern [tid:<value>] and organizes them into per-thread collections.
 * Supports filtering via predicates for advanced analysis scenarios.
 */
class ThreadBufferAnalyzer {
public:
    using ThreadId = std::string;
    using LogEntryPredicate = std::function<bool(const LogEntry&)>;
    using ThreadEntryMap = std::unordered_map<ThreadId, std::vector<LogEntry>>;
    
private:
    static const std::regex THREAD_ID_PATTERN;
    
public:
    /**
     * @brief Analyze buffer and organize entries by thread ID
     * @param buffer Buffer to analyze
     * @param predicate Optional filter predicate (default: accept all)
     * @return Map of thread ID to vector of log entries
     */
    ThreadEntryMap analyzeByThread(const ulog::LogBuffer& buffer, 
                                  LogEntryPredicate predicate = nullptr) const;
    
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
                                  LogEntryPredicate predicate = nullptr) const;
    
    /**
     * @brief Extract thread ID from log message
     * @param message Log message to parse
     * @return Thread ID if found, empty string otherwise
     */
    std::string extractThreadId(const std::string& message) const;
    
    /**
     * @brief Check if message contains thread ID pattern
     * @param message Message to check
     * @return True if thread ID pattern found
     */
    bool hasThreadId(const std::string& message) const;
    
    /**
     * @brief Get list of all unique thread IDs from buffer
     * @param buffer Buffer to analyze
     * @return Vector of unique thread IDs
     */
    std::vector<ThreadId> getThreadIds(const ulog::LogBuffer& buffer) const;
    
    /**
     * @brief Get statistics about thread distribution
     * @param thread_entries Map of thread entries
     * @return Summary statistics
     */
    struct ThreadDistributionStats {
        size_t total_threads;
        size_t total_entries;
        size_t max_entries_per_thread;
        size_t min_entries_per_thread;
        double avg_entries_per_thread;
        ThreadId most_active_thread;
        ThreadId least_active_thread;
    };
    
    ThreadDistributionStats getDistributionStats(const ThreadEntryMap& thread_entries) const;
};

} // namespace extensions
} // namespace ulog
```

### Key Features
- Regex-based thread ID extraction
- Support for filtering with predicates
- Template-based iterable analysis
- Thread distribution statistics
- Comprehensive error handling for malformed entries

### Usage Examples
```cpp
// Analyze entire buffer
ThreadBufferAnalyzer analyzer;
auto thread_entries = analyzer.analyzeByThread(logger.buffer());

// Analyze with filter (only ERROR level messages)
auto error_predicate = [](const LogEntry& entry) {
    return entry.level == LogLevel::ERROR;
};
auto error_by_thread = analyzer.analyzeByThread(logger.buffer(), error_predicate);

// Get thread distribution stats
auto stats = analyzer.getDistributionStats(thread_entries);
std::cout << "Total threads: " << stats.total_threads << std::endl;
```

## 21.1.4: Enhanced BufferStats for Iterables

### Modified BufferStats Class

```cpp
namespace ulog {
namespace extensions {

/**
 * @brief Enhanced BufferStats that works with any iterable of LogEntry
 * 
 * Extended version of BufferStats that can analyze any iterable collection
 * of LogEntry objects, not just Logger buffers. This enables per-thread
 * analysis and other advanced scenarios.
 */
class BufferStats {
private:
    // Existing private members...
    
public:
    // Existing constructors...
    
    /**
     * @brief Constructor for iterable analysis
     * @tparam Iterator Iterator type for log entries
     * @param begin Iterator to first entry
     * @param end Iterator past last entry
     */
    template<typename Iterator>
    BufferStats(Iterator begin, Iterator end);
    
    /**
     * @brief Constructor for vector analysis
     * @param entries Vector of log entries to analyze
     */
    explicit BufferStats(const std::vector<LogEntry>& entries);
    
    // All existing methods work with the new constructors...
    
private:
    /**
     * @brief Initialize stats from iterable
     * @tparam Iterator Iterator type
     * @param begin Iterator to first entry
     * @param end Iterator past last entry
     */
    template<typename Iterator>
    void initializeFromIterable(Iterator begin, Iterator end);
    
    // Make all analysis methods work with internal entry collection
    std::vector<LogEntry> entries_; // Internal storage for iterable-based analysis
    bool owns_entries_; // Flag to track if we own the entry data
};

} // namespace extensions
} // namespace ulog
```

### Enhancement Details
- Template-based constructor for any iterable
- Internal storage for entries when not using Logger buffer
- All existing methods work unchanged
- Backward compatibility maintained
- Memory-efficient for large datasets

## 21.1.5: Demo Implementation

### Demo Structure

```cpp
/**
 * @file demo_threaded_buffer_stats.cpp
 * @brief Multi-threaded demonstration of BufferStats with per-thread analysis
 */

// Demo components:
class ThreadedWorkSimulator {
public:
    struct WorkerConfig {
        std::string worker_name;
        int num_operations;
        std::chrono::milliseconds base_delay;
        std::chrono::milliseconds max_jitter;
        std::vector<std::string> operation_prefixes;
    };
    
    void simulateWork(ulog::Logger& logger, const WorkerConfig& config);
};

class MultiThreadReporter {
public:
    struct ThreadReport {
        std::string thread_id;
        size_t message_count;
        std::chrono::milliseconds total_duration;
        ulog::extensions::PerformanceMetrics performance;
        ulog::extensions::ContentionMetrics contention;
    };
    
    struct OverallReport {
        std::chrono::milliseconds total_execution_time;
        size_t total_threads;
        size_t total_log_entries;
        std::vector<ThreadReport> thread_reports;
        ThreadReport aggregated_stats;
    };
    
    OverallReport generateReport(const ThreadBufferAnalyzer::ThreadEntryMap& thread_entries);
    void printReport(const OverallReport& report);
};
```

### Demo Scenarios

1. **Database Worker Threads**
   - SQL operations with varying delays
   - Connection pool contention simulation
   - Transaction rollback scenarios

2. **Web Request Handler Threads**
   - HTTP request processing
   - Cache hit/miss patterns
   - Authentication delays

3. **Background Processing Threads**
   - File processing operations
   - Email sending queues
   - Cleanup tasks

4. **System Monitor Threads**
   - Health check operations
   - Metric collection
   - Alert generation

### Demo Flow
```cpp
void demo_multi_threaded_analysis() {
    // 1. Setup logger with thread ID observer
    auto& logger = ulog::getLogger("MultiThreadApp");
    logger.enable_buffer(10000);
    
    auto thread_observer = std::make_unique<ThreadIdObserver>(
        std::make_unique<ConsoleObserver>());
    logger.register_observer(std::move(thread_observer));
    
    // 2. Execute multiple worker threads
    std::vector<std::thread> workers;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Start various worker types
    workers.emplace_back([&] { simulate_database_workers(logger); });
    workers.emplace_back([&] { simulate_web_handlers(logger); });
    workers.emplace_back([&] { simulate_background_tasks(logger); });
    workers.emplace_back([&] { simulate_system_monitors(logger); });
    
    // 3. Wait for completion
    for (auto& worker : workers) {
        worker.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    
    // 4. Analyze results
    ThreadBufferAnalyzer analyzer;
    auto thread_entries = analyzer.analyzeByThread(logger.buffer());
    
    // 5. Generate per-thread statistics
    MultiThreadReporter reporter;
    auto overall_report = reporter.generateReport(thread_entries);
    overall_report.total_execution_time = 
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 6. Print comprehensive report
    reporter.printReport(overall_report);
}
```

## File Structure

### New Files to Create

```
demos/extensions/include/
├── thread_id_observer.h          # ThreadIdObserver implementation
├── thread_buffer_analyzer.h      # ThreadBufferAnalyzer implementation
└── multi_thread_reporter.h       # MultiThreadReporter implementation

demos/
├── demo_threaded_buffer_stats.cpp # Main demo implementation
└── include/
    └── threaded_work_simulator.h  # Work simulation helpers
```

### Modified Files

```
demos/extensions/include/
└── buffer_stats.h                # Enhanced with iterable support

demos/extensions/include/
└── buffer_stats_reporter.h       # Minor updates for thread reporting
```

## Implementation Plan

### Phase 1: Core Components
1. Implement `ThreadIdObserver` class
2. Implement `ThreadBufferAnalyzer` class
3. Add unit tests for thread ID parsing

### Phase 2: BufferStats Enhancement
1. Add iterable constructors to `BufferStats`
2. Refactor internal implementation
3. Ensure backward compatibility
4. Add tests for new functionality

### Phase 3: Reporting and Demo
1. Implement `MultiThreadReporter`
2. Create worker simulation classes
3. Build comprehensive demo
4. Add performance benchmarks

### Phase 4: Integration and Testing
1. Integration testing with existing demos
2. Performance validation
3. Documentation updates
4. Code review and optimization

## Performance Considerations

### Thread ID Extraction
- Regex compilation cached as static member
- String operations minimized
- Memory allocations reduced where possible

### Memory Usage
- Efficient storage for per-thread entries
- Copy-on-write semantics where applicable
- Automatic cleanup of temporary data

### Concurrency
- Lock-free operations where possible
- Minimal critical sections
- Thread-local storage for frequently accessed data

## Error Handling

### Malformed Thread IDs
- Graceful handling of parsing errors
- Entries without thread IDs grouped as "unknown"
- Detailed error reporting in debug mode

### Resource Management
- RAII for all resource acquisition
- Exception safety guarantees
- Proper cleanup in error scenarios

## Testing Strategy

### Unit Tests
- Thread ID parsing accuracy
- Observer message modification
- Iterable analysis correctness
- Statistical calculation validation

### Integration Tests
- Multi-threaded scenarios
- Large buffer analysis
- Performance benchmarks
- Memory usage validation

### Edge Cases
- Empty buffers
- Single-threaded scenarios
- Malformed log messages
- Very large thread counts

## Documentation

### API Documentation
- Comprehensive Doxygen comments
- Usage examples for each class
- Performance characteristics
- Thread safety guarantees

### User Guide
- Integration instructions
- Best practices
- Common pitfalls
- Troubleshooting guide

## Compatibility

### Backward Compatibility
- All existing BufferStats functionality preserved
- No changes to public APIs
- Existing demos continue to work

### Forward Compatibility
- Extensible design for future enhancements
- Plugin architecture considerations
- Version compatibility handling

This detailed design provides a comprehensive solution for todo items 21.1.1 through 21.1.4, maintaining the existing architecture while adding powerful multi-threaded analysis capabilities to the ulog library.
