# ulog Extensions

This directory contains reusable extensions for the ulog library that can be used in your own projects.

## Debug Scope Extension

**File:** `include/debug_scope.h`

The Debug Scope extension provides RAII-based automatic scope entry/exit logging functionality. It includes both a `DebugScope` class for automatic logging and a `DebugObserver` class for capturing and monitoring scope traversal.

### Features

- **RAII Scope Logging**: Automatic "Entering:" and "Exiting:" messages
- **Exception Safety**: Exit messages logged even when exceptions occur
- **Scope Tracking**: Observer to capture and monitor scope messages
- **Nested Scope Support**: Works seamlessly with nested function calls
- **Multi-Logger Support**: Each scope can use different loggers
- **Non-copyable Design**: Ensures proper RAII semantics

### Usage Example

```cpp
#include "ulog/ulog.h"
#include "extensions/include/debug_scope.h"

using ulog::extensions::DebugScope;
using ulog::extensions::DebugObserver;

auto& logger = ulog::getLogger("demo");
logger.set_log_level(ulog::LogLevel::DEBUG);

auto observer = std::make_shared<DebugObserver>("ScopeTracker");

{
    ulog::ObserverScope observerScope(logger, observer);
    
    {
        DebugScope scope(logger, "main_function");
        logger.info("Doing work in main function");
        
        {
            DebugScope innerScope(logger, "inner_function");
            logger.debug("Processing data");
        } // Automatically logs "Exiting: inner_function"
        
    } // Automatically logs "Exiting: main_function"
}

observer->printCapturedMessages();
```

### DebugScope Class

The `DebugScope` class provides automatic scope entry/exit logging:

| Method | Description |
|--------|-------------|
| `DebugScope(logger, scope_name)` | Constructor - logs entering message |
| `~DebugScope()` | Destructor - logs exiting message |
| `getScopeName()` | Get the scope name/label |
| `getLogger()` | Get reference to the logger |

### DebugObserver Class

The `DebugObserver` class captures and tracks debug scope messages:

| Method | Description |
|--------|-------------|
| `DebugObserver(name)` | Constructor with observer name |
| `getMessageCount()` | Get number of captured messages |
| `getCapturedMessages()` | Get all captured message strings |
| `printCapturedMessages()` | Print captured messages to console |
| `clearCapturedMessages()` | Clear all captured messages |
| `getName()` | Get observer name/identifier |

### Key Benefits

- **Automatic Cleanup**: RAII ensures exit messages even with exceptions
- **Zero Overhead**: No performance impact when debug level is disabled
- **Nested Support**: Handles complex call hierarchies naturally
- **Monitoring Capability**: Track program flow with observer pattern
- **Simple Integration**: Just wrap code blocks with DebugScope

## Buffer Assertions Extension

**File:** `include/buffer_assertions.h`

The BufferAssertions extension provides a comprehensive set of assertion utilities for testing log buffer contents. It's designed to be used in unit tests and demos to validate that the correct log messages were written.

### Features

- **Size Assertions**: Test buffer size, empty/non-empty state
- **Message Content**: Search for specific text in messages  
- **Log Level Filtering**: Count messages by log level
- **Pattern Matching**: Use regex patterns to validate message format
- **Custom Predicates**: Define custom validation logic
- **Chronological Order**: Verify messages are in time order
- **First/Last Message**: Test specific positions in buffer
- **Performance Testing**: Helpers for measuring logging performance

### Usage Example

```cpp
#include "ulog/ulog.h"
#include "buffer_assertions.h"

// Set up logger with buffer
auto& logger = ulog::getLogger("test");
logger.enable_buffer(100);
logger.disable_console(); // Optional: disable console for testing

// Write some messages
logger.info("User logged in");
logger.error("Database error");
logger.warn("Low memory");

// Create assertions and test
ulog::extensions::BufferAssertions assertions(logger);

// Basic assertions
assertions.assert_size(3);
assertions.assert_not_empty();

// Content assertions  
assertions.assert_contains_message("logged in");
assertions.assert_not_contains_message("nonexistent");

// Level assertions
assertions.assert_level_count(ulog::LogLevel::ERROR, 1);
assertions.assert_level_count(ulog::LogLevel::INFO, 1);

// Pattern matching
assertions.assert_contains_pattern(R"(User .+ logged in)");

// Custom predicates
assertions.assert_any_match(
    [](const ulog::LogEntry& entry) { 
        return entry.level == ulog::LogLevel::ERROR; 
    },
    "at least one error message"
);

// Order validation
assertions.assert_chronological_order();
```

### Available Assertion Methods

| Method | Description |
|--------|-------------|
| `assert_size(n)` | Assert buffer contains exactly n entries |
| `assert_empty()` | Assert buffer is empty |
| `assert_not_empty()` | Assert buffer contains entries |
| `assert_contains_message(text)` | Assert buffer contains message with text |
| `assert_not_contains_message(text)` | Assert buffer doesn't contain text |
| `assert_contains_pattern(regex)` | Assert buffer contains message matching regex |
| `assert_level_count(level, n)` | Assert exactly n entries with log level |
| `assert_chronological_order()` | Assert entries are in time order |
| `assert_first_message_contains(text)` | Assert first message contains text |
| `assert_last_message_contains(text)` | Assert last message contains text |
| `assert_all_match(predicate, desc)` | Assert all entries match predicate |
| `assert_any_match(predicate, desc)` | Assert at least one entry matches predicate |

### Helper Methods

| Method | Description |
|--------|-------------|
| `print_buffer_contents(prefix)` | Print all buffer entries for debugging |
| `get_buffer_size()` | Get current buffer size |
| `clear_buffer()` | Clear the buffer contents |

### Error Handling

All assertion methods throw `BufferAssertionException` when they fail, providing descriptive error messages that explain what was expected vs. what was found.

## Integration

To use this extension in your project:

1. Include the header: `#include "buffer_assertions.h"`
2. Make sure your logger has buffering enabled: `logger.enable_buffer(capacity)`
3. Create assertions object: `BufferAssertions assertions(logger)`
4. Use assertion methods to validate buffer contents

The extension is header-only and requires C++17 or later.

## Demo

See `../demo_buffer_assertions.cpp` for a comprehensive demonstration of all features and usage patterns.

## Buffer Statistics Extension

**Files:** `include/buffer_stats.h`, `include/buffer_stats_reporter.h`

The BufferStats extension provides comprehensive statistical analysis capabilities for log buffer contents, focusing on performance metrics, message pattern analysis, and outlier detection. Unlike BufferAssertions which validates expected conditions, BufferStats calculates and reports actual metrics without validation.

### Features

- **Basic Statistics**: Message counts, level distribution, frequency analysis
- **Timing Analysis**: Intervals, timespan, average/median calculations  
- **Performance Metrics**: Slow operation detection and analysis
- **Outlier Detection**: Identify messages followed by unusually long delays
- **Pattern Analysis**: Top message prefixes and frequency patterns
- **Contention Analysis**: Lock contention pattern detection
- **Lambda Filtering**: Custom message filtering with predicates
- **Comprehensive Reporting**: Human-readable reports with insights

### Usage Example

```cpp
#include "ulog/ulog.h"
#include "buffer_stats.h"
#include "buffer_stats_reporter.h"

// Set up logger with buffer
auto& logger = ulog::getLogger("app");
logger.enable_buffer(1000);

// Generate some messages with timing
logger.info("SQL_SELECT: user query");
std::this_thread::sleep_for(std::chrono::milliseconds(200));
logger.info("CACHE_HIT: user data found");
std::this_thread::sleep_for(std::chrono::milliseconds(10));
logger.info("AWS_S3: file upload");
std::this_thread::sleep_for(std::chrono::milliseconds(500));

// Analyze buffer statistics
ulog::extensions::BufferStats stats(logger);

// Basic statistics
std::cout << "Total messages: " << stats.total_messages() << std::endl;
std::cout << "Average interval: " << stats.average_interval().count() << "ms" << std::endl;

// Performance analysis
std::vector<std::string> slow_prefixes = {"SQL_", "AWS_"};
auto perf_metrics = stats.analyze_slow_operations(slow_prefixes);
std::cout << "Slow operations: " << perf_metrics.slow_operations_count << std::endl;

// Outlier detection
auto outliers = stats.delay_outliers(90.0); // 90th percentile
std::cout << "Delay outliers: " << outliers.size() << std::endl;

// Pattern analysis
auto top_prefixes = stats.top_prefixes(5, 15);
for (const auto& pair : top_prefixes) {
    std::cout << "Prefix \"" << pair.first << "\": " << pair.second << " times" << std::endl;
}

// Lambda filtering
auto error_messages = stats.filter_messages([](const ulog::LogEntry& entry) {
    return entry.level == ulog::LogLevel::ERROR;
});

// Generate comprehensive report
ulog::extensions::BufferStatsReporter reporter(stats);
std::cout << reporter.generate_full_report() << std::endl;
```

### Statistical Methods

| Method | Description |
|--------|-------------|
| `total_messages()` | Get total number of messages |
| `messages_by_level(level)` | Count messages by log level |
| `level_distribution()` | Get map of level -> count |
| `total_timespan()` | Duration from first to last message |
| `average_interval()` | Average time between messages |
| `median_interval()` | Median time between messages |
| `all_intervals()` | Vector of all intervals |

### Pattern Analysis

| Method | Description |
|--------|-------------|
| `top_prefixes(n, length)` | Most frequent message prefixes |
| `message_frequency(n)` | Most frequent complete messages |

### Performance Analysis

| Method | Description |
|--------|-------------|
| `analyze_slow_operations(prefixes)` | Analyze operations with specific prefixes |
| `analyze_contention(prefixes)` | Analyze lock contention patterns |
| `delay_outliers(percentile)` | Find messages with unusual delays |

### Advanced Filtering

| Method | Description |
|--------|-------------|
| `filter_messages(predicate)` | Filter messages with custom predicate |
| `top_messages_by(n, extract, comp)` | Top N messages by custom criteria |

### Reporting

The `BufferStatsReporter` class generates human-readable reports:

| Method | Description |
|--------|-------------|
| `generate_summary_report()` | Basic statistics summary |
| `generate_performance_report()` | Performance-focused analysis |
| `generate_outlier_report()` | Outlier detection results |
| `generate_full_report()` | Comprehensive analysis |
| `print_report(stream)` | Print report to stream |
| `save_report(filename)` | Save report to file |

### Report Insights

The reporter provides actionable insights such as:
- "Top 5 slowest operation types contributing to 80% of total delay time"
- "Lock contention detected with 300% variance in delay times"
- "SQL operations show consistent 200-400ms delays"
- "3 outlier operations detected exceeding 90th percentile by >5x"

## Integration

To use these extensions in your project:

1. Include the headers: `#include "buffer_stats.h"` and `#include "buffer_stats_reporter.h"`
2. Make sure your logger has buffering enabled: `logger.enable_buffer(capacity)`
3. Create stats object: `BufferStats stats(logger)`
4. Use statistical methods or create reporter: `BufferStatsReporter reporter(stats)`

Both extensions are header-only and require C++17 or later.

## Demos

- See `../demo_buffer_assertions.cpp` for BufferAssertions usage
- See `../demo_buffer_stats.cpp` for BufferStats comprehensive scenarios
