# ulog - Lightweight C++ Logging Library

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A fast, simple, and lightweight header-only C++ logging library with console output and optional memory buffer support.

## Features

- **Header-only**: Single header file, no compilation required
- **Fast & Lightweight**: <1000 LOC, optimized for speed and simplicity
- **Thread-safe**: All operations are thread-safe
- **Flexible formatting**: Support for anonymous (`{?}`) and positional (`{0}`, `{1}`) parameters
- **Memory buffer**: Optional in-memory log storage with configurable capacity
- **Observer pattern**: Extensible through custom log observers
- **Console control**: Enable/disable console output at runtime
- **RAII support**: Automatic resource management for observers
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
    ulog::observer_scope scope(logger, observer);
    logger.info("This will be observed");
} // Observer automatically removed
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

## Installation

### Method 1: Header-only

Simply copy `include/ulog/ulog.h` to your project and include it:

```cpp
#include "ulog/ulog.h"
```

### Method 2: CMake Integration

Clone the repository and use CMake:

```bash
git clone https://github.com/your-repo/ulog.git
cd ulog
mkdir build && cd build
cmake ..
make
```

### Method 3: CMake FetchContent

Add to your `CMakeLists.txt`:

```cmake
include(FetchContent)
FetchContent_Declare(
    ulog
    GIT_REPOSITORY https://github.com/your-repo/ulog.git
    GIT_TAG main
)
FetchContent_MakeAvailable(ulog)

target_link_libraries(your_target ulog)
```

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

# Run demo application
./run_demo.sh

# Generate Doxygen documentation (requires Doxygen)
./build_docs.sh
```

### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# Run tests
./ulog_tests

# Run demo
./ulog_demo
```

## API Reference

### Core Classes

#### `ulog::Logger`

Main logging class with the following methods:

- `trace()`, `debug()`, `info()`, `warn()`, `error()`, `fatal()` - Log messages at different levels
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
- `message` - The formatted log message
- `formatted_message()` - Get fully formatted log line

#### `ulog::LogObserver`

Abstract base class for log observers:

- `handleRegistered()` - Called when observer is added to logger
- `handleUnregistered()` - Called when observer is removed from logger
- `handleNewMessage()` - Called for each new log message
- `handleFlush()` - Called when logger is flushed

#### `ulog::observer_scope`

RAII class for automatic observer management:

```cpp
ulog::observer_scope scope(logger, observer); // Adds observer
// ... observer automatically removed when scope ends
```

### Global Functions

- `ulog::getLogger()` - Get global logger
- `ulog::getLogger(name)` - Get named logger
- `ulog::getLogger(name, factory)` - Get logger using factory function
- `ulog::getLogger(factory)` - Get global logger using factory function

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

## Examples

See the `demos/demo_main.cpp` file for comprehensive examples covering:

- Basic logging with all levels
- Parameter formatting (anonymous and positional)
- Memory buffer usage
- Observer pattern implementation
- Console control
- Thread safety demonstration
- Logger factory usage

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

*For detailed API documentation, run `./scripts/build_docs.sh` to generate Doxygen documentation.*
