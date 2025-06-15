# ulog - Lightweight C++ Logging Library

[![Build Status](https://github.com/vpiotr/ulog/actions/workflows/ci.yml/badge.svg)](https://github.com/vpiotr/ulog/actions/workflows/ci.yml)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A fast, simple, and lightweight header-only C++ logging library with console output and optional memory buffer support.

## Features

- **Header-only**: Single header file, no compilation required
- **Fast & Lightweight**: <1000 LOC, optimized for speed and simplicity
- **Thread-safe**: All operations are thread-safe
- **Log level filtering**: Built-in filtering by severity level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, OFF)
- **Flexible formatting**: Support for anonymous (`{?}`) and positional (`{0}`, `{1}`) parameters
- **Memory buffer**: Optional in-memory log storage with configurable capacity
- **Observer pattern**: Extensible through custom log observers
- **Console control**: Enable/disable console output at runtime
- **RAII support**: Automatic resource management for observers and auto-flushing scopes
- **Cross-platform**: Works on Linux, Windows, and macOS

## Quick Start

### Basic Usage

```cpp
#include "ulog/ulog.h"

int main() {
    // Get global logger
    auto logger = ulog::getLogger();
    logger.info("Hello, world!");
    
    // Get named logger
    auto appLogger = ulog::getLogger("MyApp");
    appLogger.debug("Debug message: {0}", value);
    
    return 0;
}
```

### Parameter Formatting

```cpp
auto logger = ulog::getLogger("App");

// Anonymous parameters
logger.info("User: {?}, Age: {?}", "John", 25);
// Output: 2025-06-12 10:30:15.123 [INFO] [App] User: John, Age: 25

// Positional parameters
logger.info("Name: {0}, Age: {1}, Name again: {0}", "Alice", 30);
// Output: 2025-06-12 10:30:15.124 [INFO] [App] Name: Alice, Age: 30, Name again: Alice
```

### Memory Buffer

```cpp
auto logger = ulog::getLogger("BufferedApp");

// Enable buffer with capacity of 100 messages (0 = unlimited)
logger.enable_buffer(100);

logger.info("This message is buffered");
logger.debug("This too");

// Access buffer contents
auto buffer = logger.buffer();
for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
    // it->message contains formatted message with parameters substituted: "This message is buffered"
    // it->formatted_message() contains full log line: "2025-06-15 10:30:15.123 [INFO] [BufferedApp] This message is buffered"
    std::cout << it->formatted_message() << std::endl;
}

logger.clear_buffer();
logger.disable_buffer();
```

### Observer Pattern

```cpp
class CustomObserver : public ulog::LogObserver {
public:
    void handleNewMessage(const ulog::LogEntry& entry) override {
        // Custom handling logic
        std::cout << "Observed: " << entry.message << std::endl;
    }
};

auto logger = ulog::getLogger("ObservedApp");
auto observer = std::make_shared<CustomObserver>();

// Manual observer management
logger.add_observer(observer);
logger.info("This will be observed");
logger.remove_observer(observer);

// RAII observer management
{
    ulog::ObserverScope scope(logger, observer);
    logger.info("This will be observed");
} // Observer automatically removed
```

### Auto-Flushing Scope

Use RAII to automatically flush loggers when scopes exit:

```cpp
auto logger = ulog::getLogger("FlushApp");

{
    ulog::AutoFlushingScope scope(logger);
    
    logger.info("Message 1");
    logger.warn("Message 2");
    
    // flush() will be called automatically when scope exits
}

// Nested scopes work too
{
    ulog::AutoFlushingScope outerScope(logger);
    {
        ulog::AutoFlushingScope innerScope(logger);
        logger.info("Inner message");
    } // Inner flush happens here
    logger.info("Outer message");
} // Outer flush happens here
```

### Console Control

```cpp
auto logger = ulog::getLogger("ControlApp");

logger.info("This appears on console");

logger.disable_console();
logger.info("This does not appear on console");

logger.enable_console();
logger.info("This appears on console again");
```

### Log Level Filtering

Control which messages are logged based on their severity level:

```cpp
auto logger = ulog::getLogger("FilterApp");

// Default level is INFO (logs INFO, WARN, ERROR, FATAL)
logger.trace("This will NOT appear");  // Filtered out
logger.debug("This will NOT appear");  // Filtered out
logger.info("This will appear");
logger.warn("This will appear");

// Set to TRACE to log all messages
logger.set_log_level(ulog::LogLevel::TRACE);
logger.trace("Now this will appear");
logger.debug("And this will appear");

// Set to ERROR to log only ERROR and FATAL
logger.set_log_level(ulog::LogLevel::ERROR);
logger.info("This will NOT appear");   // Filtered out
logger.error("This will appear");

// Set to OFF to disable all logging
logger.set_log_level(ulog::LogLevel::OFF);
logger.fatal("Even this won't appear"); // All messages filtered

// Check current level
LogLevel current = logger.get_log_level();
```

Available log levels (in order of severity):
- `OFF` - No logging
- `TRACE` - Most verbose
- `DEBUG` - Debug information
- `INFO` - General information (default)
- `WARN` - Warnings
- `ERROR` - Errors
- `FATAL` - Fatal errors

## Installation

### Method 1: Header-only

Simply copy `include/ulog/ulog.h` to your project and include it:

```cpp
#include "ulog/ulog.h"
```

### Method 2: CMake Integration

Clone the repository and use CMake:

```bash
git clone https://github.com/vpiotr/ulog.git
```

Then, in your `CMakeLists.txt`:

```cmake
# Add ulog as a subdirectory
add_subdirectory(path/to/ulog)

# Link against the ulog library
target_link_libraries(your_target PRIVATE ulog)
```

## Custom `to_string` with `ustr.h` (Optional)

`ulog` can be configured to use an external `to_string` implementation from a library like `ustr.h`. This is useful if you already have a `to_string` utility or want more control over string conversions, especially for custom types or advanced formatting of STL containers.

**To enable this feature:**

1.  **Define `ULOG_USE_USTR`**: Before including `ulog/ulog.h`, define the macro `ULOG_USE_USTR`.
    ```cpp
    #define ULOG_USE_USTR
    #include "ulog/ulog.h"
    #include "ustr/ustr.h" // Your ustr.h header
    ```

2.  **Provide `ustr.h`**: Ensure that a header file named `ustr.h` is available in your include paths. This file should contain a namespace `ustr` with `to_string` template functions and specializations.

    A minimal `ustr.h` stub might look like this (see `demos/ustr/ustr.h` for a more complete example):
    ```cpp
    // demos/ustr/ustr.h (example stub)
    #pragma once
    #include <string>
    #include <sstream> // For std::ostringstream
    // Add includes for types you want to support, e.g., <vector>, <map>

    namespace ustr {

    template <typename T>
    inline std::string to_string(const T& value) {
        std::ostringstream oss;
        oss << value; // Default implementation
        return oss.str();
    }

    // Add specializations as needed
    // inline std::string to_string(const std::vector<int>& vec) { ... }

    } // namespace ustr
    ```

3.  **Link `ulog`**: When `ULOG_USE_USTR` is defined, `ulog` will call `::ustr::to_string()` instead of its internal `ulog::ustr::to_string()`.

**Example Usage:**

```cpp
#define ULOG_USE_USTR
#include "ulog/ulog.h"
#include "path/to/your/ustr.h" // Make sure this path is correct

// Example with a C-style array (assuming ustr.h has an overload for it)
int main() {
    auto logger = ulog::getLogger("UstrApp");
    int c_array[] = {10, 20, 30};
    logger.info("C-style array: {?}", c_array); 
    // If ustr::to_string supports arrays, it will be formatted accordingly.

    std::vector<std::string> my_vector = {"hello", "ustr"};
    logger.info("STL container (vector): {?}", my_vector);
    // If ustr::to_string supports std::vector<std::string>, it will be used.
    return 0;
}
```

Refer to the `demos/demo_ustr_integration.cpp` and `demos/ustr/ustr.h` for a runnable example.

## Building and Testing

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or later
- Threads library (usually available by default)

### Build Scripts

The project includes convenient shell scripts for common operations:

```bash
# Rebuild entire project (tests and demos)
./rebuild.sh

# Run all tests
./run_tests.sh

# Run demo applications
./run_demos.sh

# Run performance benchmarks
./run_benchmarks.sh

# Generate Doxygen documentation (requires Doxygen)
./build_docs.sh
```

### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# Run tests (multiple test executables)
./test_logger
./test_buffer
./test_formatter
./test_observer

# Run demo
./ulog_demo
```

## API Reference

### Core Classes

#### `ulog::Logger`

Main logging class with the following methods:

- `trace()`, `debug()`, `info()`, `warn()`, `error()`, `fatal()` - Log messages at different levels
- `set_log_level(LogLevel level)` - Set minimum log level filter
- `get_log_level()` - Get current log level filter
- `enable_buffer(size_t capacity)` - Enable memory buffer with optional capacity limit
- `disable_buffer()` - Disable memory buffer
- `clear_buffer()` - Clear buffer contents
- `enable_console()` / `disable_console()` - Control console output
- `flush()` - Flush console output
- `add_observer()` / `remove_observer()` - Manage observers
- `buffer()` - Get read-only access to buffer

#### `ulog::LogEntry`

Structure containing log information:

- `timestamp` - When the log entry was created
- `level` - Log level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
- `logger_name` - Name of the logger that created this entry
- `message` - The formatted message with all parameters already substituted
- `formatted_message()` - Get fully formatted log line with timestamp, level, logger name, and message

**Key Difference:**
- `message` contains the formatted message with parameters substituted (e.g., for `logger.info("Hello user {0}", "tom123")`, this would be "Hello user tom123")
- `formatted_message()` returns the complete formatted log line (e.g., "2025-06-15 10:30:15.123 [INFO] [MyApp] Hello user tom123")

**Parameter Processing:**
When you call `logger.info("Hello user {0}", "tom123")`, the parameters are processed as follows:
1. Parameters are converted to strings using `to_string()`
2. Placeholders (`{0}`, `{1}`, `{?}`) are replaced with the converted parameter values
3. The resulting formatted message ("Hello user tom123") is stored in the `message` field
4. Original format string and parameters are not preserved in the LogEntry

#### `ulog::LogObserver`

Abstract base class for log observers:

- `handleRegistered()` - Called when observer is added to logger
- `handleUnregistered()` - Called when observer is removed from logger
- `handleNewMessage()` - Called for each new log message
- `handleFlush()` - Called when logger is flushed

#### `ulog::ObserverScope`

RAII class for automatic observer management:

```cpp
ulog::ObserverScope scope(logger, observer); // Adds observer
// ... observer automatically removed when scope ends
```

#### `ulog::AutoFlushingScope`

RAII class for automatic logger flushing:

```cpp
ulog::AutoFlushingScope scope(logger); // Will flush when scope ends
logger.info("This message will be flushed automatically");
// ... logger flushed when scope ends
```

### Global Functions

- `ulog::getLogger()` - Get global logger
- `ulog::getLogger(name)` - Get named logger
- `ulog::getLogger(name, factory)` - Get logger using factory function
- `ulog::getLogger(factory)` - Get global logger using factory function

## Custom Type Support

ulog can log any type that supports stream output (`operator<<`). For custom classes, simply provide an `operator<<` overload:

```cpp
class Person {
public:
    Person(const std::string& name, int age) : name_(name), age_(age) {}
    
    // Provide operator<< for ulog support
    friend std::ostream& operator<<(std::ostream& os, const Person& person) {
        os << "Person(name=" << person.name_ << ", age=" << person.age_ << ")";
        return os;
    }
    
private:
    std::string name_;
    int age_;
};

int main() {
    auto logger = ulog::getLogger("CustomDemo");
    
    Person person("Alice", 30);
    logger.info("Created user: {?}", person);
    // Output: 2025-06-12 10:30:15.123 [INFO] [CustomDemo] Created user: Person(name=Alice, age=30)
    
    return 0;
}
```

For comprehensive examples including advanced formatting, template specialization, container support, and performance tips, see `demos/demo_custom_formatting.cpp`.

## Output Format

All log messages follow this consistent format:

```
<timestamp> [<log-level>] [<logger-name>] <message>
```

Example:
```
2025-06-12 10:30:15.123 [INFO] [MyApp] Application started successfully
2025-06-12 10:30:15.124 [DEBUG] [Database] Connected to database: localhost:5432
2025-06-12 10:30:15.125 [WARN] [Cache] Cache miss for key: user_123
2025-06-12 10:30:15.126 [ERROR] [Network] Failed to connect to remote service
```

For global logger (empty name), the logger name part is omitted:
```
2025-06-12 10:30:15.127 [INFO] Global message without logger name
```

## Performance Characteristics

- **Minimal overhead**: Header-only design with inline optimizations
- **Thread-safe**: Uses efficient locking mechanisms
- **Memory efficient**: Optional buffering with configurable limits
- **Format optimization**: Fast parameter substitution
- **Compile-time optimization**: Template-based design for optimal performance

## Optional Mutex Configuration

ulog provides compile-time control over mutex usage for fine-tuning performance vs. thread-safety:

### Configuration Macros

- `ULOG_USE_MUTEX_FOR_CONSOLE` - Controls console output mutex (default: 1/enabled)
- `ULOG_USE_MUTEX_FOR_BUFFER` - Controls buffer operations mutex (default: 1/enabled)
- `ULOG_USE_MUTEX_FOR_OBSERVERS` - Controls observer operations mutex (default: 1/enabled)

### Usage

Define these macros **before** including `ulog.h`:

```cpp
#define ULOG_USE_MUTEX_FOR_CONSOLE 0   // Disable console mutex
#define ULOG_USE_MUTEX_FOR_BUFFER 0    // Disable buffer mutex
#define ULOG_USE_MUTEX_FOR_OBSERVERS 0 // Disable observer mutex
#include "ulog/ulog.h"
```

### Performance Trade-offs

**With Mutexes (Default - Thread-Safe):**
- ✅ Thread-safe console output
- ✅ Thread-safe buffer operations  
- ✅ Thread-safe observer notifications
- ✅ No data races or corruption
- ⚠️ Slight performance overhead in single-threaded scenarios

**Without Mutexes (Performance Optimized):**
- ✅ Maximum performance in single-threaded scenarios
- ⚠️ **NOT thread-safe** - only use in single-threaded applications
- ⚠️ Undefined behavior if used from multiple threads

### Benchmark Results

Use the provided benchmark demos to compare performance:

Use the provided benchmark script to compare performance:

```bash
# Run all benchmarks
./run_benchmarks.sh

# Or build and run specific benchmarks manually
cd build
make demo_buffer_benchmark_with_mutex demo_buffer_benchmark_no_mutex
make demo_observer_benchmark_with_mutex demo_observer_benchmark_no_mutex

# Buffer performance benchmarks
./demo_buffer_benchmark_with_mutex     # With mutex protection
./demo_buffer_benchmark_no_mutex       # Without mutex protection

# Observer performance benchmarks  
./demo_observer_benchmark_with_mutex   # With observer mutex protection
./demo_observer_benchmark_no_mutex     # Without observer mutex protection
```

**Note:** Observer registry management (add/remove observers) uses mutex protection based on the `ULOG_USE_MUTEX_FOR_OBSERVERS` setting.

## Examples

See the `demos/` directory for comprehensive examples:

- `demos/demo_main.cpp` - Core functionality including basic logging, parameter formatting, memory buffer usage, observer pattern, console control, thread safety, and logger factory usage
- `demos/demo_file_observer.cpp` - File output via observer pattern with RAII management and multiple observers
- `demos/demo_log_level_filtering.cpp` - Log level filtering examples with buffers and observers
- `demos/demo_custom_formatting.cpp` - Custom formatting for both primitive and user-defined types including wrapper classes, operator<< overloads, template specialization, container support, and performance tips
- `demos/demo_container_formatting.cpp` - Advanced container formatting examples with STL containers and custom types
- `demos/demo_auto_flushing.cpp` - RAII auto-flushing scope examples including basic usage, nested scopes, multiple loggers, and exception safety
- `demos/demo_debug_scope.cpp` - DebugScope RAII pattern with observer integration for automatic scope entry/exit logging, nested scopes, multiple loggers, and exception safety
- `demos/demo_cerr_observer.cpp` - Error message redirection to stderr via observer pattern with multiple observer support, RAII management, and exception safety
- `demos/demo_exception_formatting.cpp` - Automatic exception formatting with custom exception wrappers, nested exception handling, system error integration, and real-world scenarios
- `demos/demo_ustr_integration.cpp` - Integration with external ustr.h library for enhanced string conversion capabilities

### Performance Benchmarks

- `demos/benchmarks/demo_buffer_benchmark_with_mutex.cpp` - Buffer write performance benchmark with mutex protection enabled
- `demos/benchmarks/demo_buffer_benchmark_no_mutex.cpp` - Buffer write performance benchmark with mutex protection disabled (single-threaded only)
- `demos/benchmarks/demo_observer_benchmark_with_mutex.cpp` - Observer notification performance benchmark with mutex protection enabled
- `demos/benchmarks/demo_observer_benchmark_no_mutex.cpp` - Observer notification performance benchmark with mutex protection disabled (single-threaded only)

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines

- Follow SOLID and DRY principles
- Maintain the <1000 LOC limit (excluding comments)
- Ensure thread safety for all public APIs
- Add comprehensive tests for new features
- Update documentation for API changes
- Maintain cross-platform compatibility

## Design Philosophy

ulog is designed with the following principles:

1. **Simplicity**: Minimal API surface, easy to learn and use
2. **Performance**: Zero-overhead abstractions where possible
3. **Flexibility**: Extensible through observers and factories
4. **Reliability**: Thread-safe, exception-safe, and tested
5. **Portability**: Works across different platforms and compilers

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with modern C++17 features
- Inspired by popular logging frameworks like spdlog and log4cpp
- Uses utest framework for comprehensive testing
- Thanks to all contributors and users

---

*For detailed API documentation, run `./build_docs.sh` to generate Doxygen documentation.*
