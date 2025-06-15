# ulog Extensions

This directory contains reusable extensions for the ulog library that can be used in your own projects.

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
