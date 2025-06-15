# ulog Demos

This directory contains demonstration applications showcasing various features of the ulog library.

## Available Demos

### 1. demo_main.cpp
The main demo application showcasing core ulog features:
- Basic logging functionality
- Memory buffer usage
- Observer pattern
- Console control
- Thread safety
- Logger factory pattern

### 2. demo_file_observer.cpp
**File Output via Observer Demo** - Implementation of the first todo item.

This demo showcases how to implement file output functionality using the observer pattern. It demonstrates:

#### Features:
- **FileObserver Class**: A custom observer that writes log entries to files
- **Basic File Logging**: Simple file output with automatic file creation
- **RAII Observer Management**: Automatic observer registration/unregistration using `ObserverScope`
- **Multiple File Observers**: Writing to multiple files simultaneously
- **Filtered Logging**: Custom observers that filter messages by log level
- **Thread-Safe File Operations**: All file operations are protected with mutexes

#### Demo Scenarios:

1. **Basic File Output**
   - Creates a `FileObserver` that writes to `demo_basic.log`
   - Demonstrates manual observer management (add/remove)
   - Shows console vs file output separation

2. **RAII Observer Management**
   - Uses `ObserverScope` for automatic observer lifecycle management
   - Writes to `demo_raii.log` in append mode
   - Observer is automatically removed when scope ends

3. **Multiple File Observers**
   - Demonstrates multiple observers on the same logger
   - Creates general log file with all messages
   - Creates filtered log file with only ERROR and FATAL messages
   - Shows how to create custom observer subclasses

#### File Observer Features:

- **File Modes**: Support for both append and overwrite modes
- **Error Handling**: Proper exception handling for file operations
- **Metadata Logging**: Logs observer registration/unregistration events
- **Thread Safety**: All file operations are thread-safe
- **Immediate Flush**: Ensures log entries are written immediately to disk

### 3. demo_log_level_filtering.cpp
**Log Level Filtering Demo** - Demonstrates log level filtering functionality.

This demo showcases how to use log level filtering to control which messages are logged:

#### Features:
- **Basic Log Level Filtering**: Shows how different log levels filter messages
- **Buffer Integration**: Demonstrates filtering with memory buffers
- **Observer Integration**: Shows how filtering affects observers
- **Level Configuration**: Examples of setting different log levels

### 4. demo_custom_formatting.cpp
**Custom Formatting Demo** - Shows how to enable custom formatting for both primitive and user-defined types.

This demo demonstrates various approaches to customize formatting in ulog:

#### Features:
- **Primitive Custom Formatting**: Wrapper classes for hex, percentage, precision control
- **Basic Custom Classes**: Simple classes with `operator<<` overload
- **Advanced Formatting**: JSON-like output for complex objects
- **Geometric Types**: Coordinate and mathematical objects
- **Specialization**: Using `ustr::to_string` template specialization
- **Container Support**: Approaches for logging standard containers
- **Performance Tips**: Best practices for efficient custom type logging

#### Demo Scenarios:

1. **Basic Custom Class Support**
   - Person class with name and age
   - Simple operator<< implementation
   - Integration with ulog parameter formatting

2. **Advanced Custom Class (JSON-like)**
   - UserAccount class with complex data
   - Structured JSON-style output
   - Multiple data fields formatting

3. **Geometric Types**
   - Point class with coordinates
   - Mathematical calculations and logging
   - Coordinate system representations

4. **ustr::to_string Specialization**
   - Alternative to operator<< approach
   - Template specialization technique
   - Full control over string conversion

5. **Container Support**
   - Manual formatting for std::vector and std::map
   - Helper functions for container logging
   - Individual element access patterns

6. **Performance Considerations**
   - Efficient logging practices
   - Log level checking for expensive operations
   - String conversion optimization tips

### 5. demo_container_formatting.cpp
**Container Formatting Demo** - Demonstrates manual formatting approaches for standard containers.

This demo showcases recommended patterns for logging standard library containers using helper functions. Since ulog focuses on performance and doesn't include built-in container formatting, this demo provides practical examples of manual formatting approaches:

#### Features:
- **Helper Function Pattern**: Demonstrates the recommended approach using manual formatting functions
- **Standard Container Support**: Examples for std::vector, std::pair, and std::map
- **Nested Container Handling**: Shows how to format complex nested containers like vector of pairs
- **Performance-Oriented**: Manual approach avoids overhead of automatic container detection
- **Extensible Pattern**: Easy to extend for additional container types
- **Step-by-Step Approach**: Clear demonstration of building container formatting capabilities

#### Demo Scenarios:

1. **Vector Formatting**
   - std::vector<int> with bracket notation [1, 2, 3, 4, 5]
   - Template-based helper function for type flexibility
   - Comma-separated element display

2. **Pair Formatting**
   - std::pair<int, std::string> with parentheses notation (42, "Alice")
   - Generic template supporting different pair types
   - Clean key-value representation

3. **Map Formatting**
   - std::map<std::string, int> with JSON-like notation {"Alice": 95, "Bob": 87}
   - Iterator-based approach for efficient traversal
   - Key-value separator formatting

4. **Nested Container Formatting**
   - std::vector<std::pair<std::string, int>> combining multiple patterns
   - Demonstrates composition of formatting functions
   - Complex data structure representation

5. **Individual Element Access**
   - Shows alternative approach using structured bindings
   - Demonstrates when individual logging might be preferred
   - Performance comparison with bulk formatting

#### Helper Function Examples:

```cpp
// Vector formatting
template<typename T>
std::string format_vector(const std::vector<T>& vec);

// Pair formatting  
template<typename T1, typename T2>
std::string format_pair(const std::pair<T1, T2>& p);

// Map formatting
template<typename Key, typename Value>
std::string format_map(const std::map<Key, Value>& m);

// Nested container formatting
template<typename T1, typename T2>
std::string format_vector_of_pairs(const std::vector<std::pair<T1, T2>>& vec);
```

#### Use Cases:
- **Data Structure Debugging**: Quick visualization of container contents
- **Algorithm Verification**: Checking intermediate results in data processing
- **Configuration Logging**: Displaying settings stored in containers
- **Performance Analysis**: Logging data structure states for optimization

### 6. demo_auto_flushing.cpp
**AutoFlushingScope RAII Demo** - Demonstrates automatic logger flushing using RAII pattern.

This demo showcases the AutoFlushingScope class which provides automatic flushing of loggers when scopes exit, ensuring log data is persisted even in the presence of exceptions or early returns:

#### Features:
- **RAII Auto-Flushing**: Automatic logger flushing when scope exits
- **Exception Safety**: Ensures flushing occurs even during exception unwinding
- **Nested Scope Support**: Proper handling of multiple nested AutoFlushingScope instances
- **Multiple Logger Support**: Independent flushing for different loggers
- **FlushObserver Integration**: Demonstrates tracking of flush operations for debugging
- **Zero Overhead**: No performance impact when auto-flushing is not needed

#### Demo Scenarios:

1. **Basic AutoFlushingScope Usage**
   - Simple scope creation with automatic entry/exit flushing
   - FlushObserver tracks flush operations for verification
   - RAII ensures proper cleanup without manual flush calls

2. **Nested AutoFlushingScope Usage**
   - Demonstrates multiple levels of nested scopes
   - Each scope independently flushes its logger on exit
   - Proper ordering and counting of flush operations

3. **Multiple Logger AutoFlushingScope**
   - Uses different AutoFlushingScope instances for different loggers
   - Independent flushing behavior per logger
   - Separate flush tracking for each logger

4. **Exception Safety Testing**
   - Tests RAII behavior during exception handling
   - Ensures flush occurs despite thrown exceptions
   - Demonstrates robustness in error conditions

5. **FlushObserver Integration**
   - Custom observer class for tracking flush operations
   - Flush counting and verification capabilities
   - Debugging support for flush behavior analysis

#### AutoFlushingScope Features:

- **Automatic Lifecycle Management**: Constructor/destructor handle flush timing
- **Exception Safe**: Uses RAII to guarantee flushing even during stack unwinding
- **Lightweight**: Minimal overhead when auto-flushing is not required
- **Thread Safe**: Works correctly with ulog's thread-safe logger implementation
- **Composable**: Can be combined with other RAII patterns and observers

#### Use Cases:
- **Critical Section Logging**: Ensure logs are persisted before leaving important code sections
- **Error Recovery**: Guarantee log data is written before error handling begins
- **Transaction Boundaries**: Flush logs at the end of logical transactions
- **Resource Management**: Ensure logging data persistence alongside other resource cleanup
- **Testing and Debugging**: Verify flush behavior and timing in unit tests

### 7. demo_debug_scope.cpp
**DebugScope RAII Demo** - Demonstrates the DebugScope pattern with observer integration.

This demo showcases a custom DebugScope class that automatically logs "Entering: x" and "Exiting: x" messages for labeled scopes using RAII pattern combined with observer management:

#### Features:
- **DebugScope Class**: RAII class that logs scope entry and exit automatically
- **Observer Integration**: Custom DebugObserver that tracks scope-related messages
- **Nested Scope Support**: Demonstrates deeply nested scope logging
- **Multiple Logger Support**: Shows scope tracking across different loggers
- **Exception Safety**: Ensures exit logging even when exceptions occur
- **Conditional Debugging**: Shows how log level filtering affects scope visibility

#### Demo Scenarios:

1. **Basic DebugScope Usage**
   - Simple scope creation with automatic entry/exit logging
   - Observer captures and displays scope messages
   - RAII ensures proper cleanup

2. **Nested DebugScope Usage**
   - Demonstrates multiple levels of nested scopes
   - Shows proper ordering of entry/exit messages
   - Tracks scope depth and hierarchy

3. **Multiple Logger DebugScope**
   - Uses different loggers for different services
   - Independent scope tracking per logger
   - Separate observers for each logger

4. **Exception Safety**
   - Tests RAII behavior during exception handling
   - Ensures exit messages are logged despite exceptions
   - Demonstrates robustness of scope tracking

5. **Conditional DebugScope**
   - Shows effect of log level filtering on scope visibility
   - Demonstrates when scope messages appear/disappear
   - Useful for production vs debug environments

### 8. demo_cerr_observer.cpp
**Cerr Observer Demo** - Demonstrates logging errors to stderr via observer.

This demo showcases how to create a custom observer that filters error messages and redirects them to stderr (standard error) while allowing normal messages to go to stdout (standard output):

#### Features:
- **CerrObserver Class**: Custom observer that filters ERROR and FATAL messages to stderr
- **Error Stream Separation**: Normal messages go to stdout, errors go to stderr
- **RAII Observer Management**: Automatic observer registration/unregistration
- **Multiple Observer Support**: Combines cerr observer with other observers
- **Log Level Filtering**: Works correctly with logger-level filtering
- **Exception Safety**: Ensures proper cleanup during exception handling

#### Demo Scenarios:

1. **Basic Cerr Observer Usage**
   - Creates observer that redirects ERROR and FATAL messages to stderr
   - Shows separation between normal output (stdout) and error output (stderr)
   - Manual observer management with add/remove

2. **RAII Cerr Observer Management**
   - Uses `ObserverScope` for automatic observer lifecycle management
   - Demonstrates scope-based error redirection
   - Observer automatically removed when scope ends

3. **Multiple Observers with Cerr**
   - Combines cerr observer with message counter observer
   - Shows multiple observers working together
   - Statistics tracking alongside error redirection

4. **Log Level Filtering with Cerr**
   - Tests cerr observer behavior with different log levels
   - Shows that filtering happens before observer notification
   - Demonstrates proper interaction between filtering and stderr redirection

5. **Exception Safety**
   - Tests RAII behavior during exception handling
   - Ensures stderr redirection works despite exceptions
   - Demonstrates robustness of observer management

#### Use Cases:
- **Error Stream Separation**: Allows tools to capture errors separately from normal output
- **Log Analysis**: Enables separate processing of error and normal log streams
- **Production Monitoring**: Facilitates error-specific alerting and monitoring
- **Development Debugging**: Helps developers focus on error output during debugging

### 9. demo_exception_formatting.cpp
**Exception Formatting Demo** - Demonstrates automatic formatting of exceptions for enhanced error logging.

This demo showcases how to create rich, informative exception logs using custom formatting patterns, enabling detailed error analysis and debugging:

#### Features:
- **ExceptionFormatter Wrapper**: Custom wrapper for automatic exception formatting with configurable options
- **Custom Exception Types**: Examples of creating informative custom exception classes with additional context
- **Nested Exception Support**: Handling and formatting exception chains with complete causality information
- **System Error Integration**: Formatting std::system_error and std::filesystem_error types
- **Real-World Scenarios**: Practical examples including database errors, network failures, and resource management
- **Performance Considerations**: Best practices for efficient exception logging in production code

#### Demo Scenarios:

1. **Basic Exception Formatting**
   - Standard exception types (runtime_error, invalid_argument, out_of_range)
   - Configurable formatting options (type, message, nested exceptions)
   - Simple wrapper usage for immediate enhanced logging

2. **Custom Exception Types**
   - DatabaseError with error codes and connection strings
   - NetworkError with host, port, and timeout information
   - Specialized formatting using operator<< and ustr::to_string

3. **Nested Exception Handling**
   - Exception chains with std::throw_with_nested
   - Recursive nested exception unwrapping and formatting
   - Complete error causality tracking for complex failures

4. **System Error Integration**
   - std::system_error formatting with error codes
   - std::filesystem_error with path and operation context
   - Platform-specific error message extraction

5. **Real-World Application Scenarios**
   - Resource initialization failures with fallback strategies
   - API call retries with progressive error accumulation
   - Data processing pipelines with context-aware error reporting

6. **Performance Optimization**
   - Log level checking before expensive exception formatting
   - Efficient formatter creation and reuse strategies
   - Best practices for production exception logging

#### Use Cases:
- **Enhanced Error Debugging**: Rich exception information for faster problem resolution
- **Production Monitoring**: Detailed error context for automated alerting and analysis
- **Error Analytics**: Structured exception data for trend analysis and root cause identification
- **Multi-tier Applications**: Exception propagation with preserved context across service boundaries

### 10. demo_buffer_assertions.cpp
**Buffer Assertions Extension Demo** - Demonstrates comprehensive testing utilities for validating log buffer contents.

This demo showcases the BufferAssertions extension, which provides a rich set of assertion utilities for testing log buffer contents in unit tests and demos:

#### Features:
- **BufferAssertions Extension**: Comprehensive assertion library for buffer content validation
- **Test Helper Classes**: Reusable RAII test utilities for consistent test setup and teardown
- **Performance Testing**: Built-in helpers for measuring logging and assertion performance
- **Error Handling**: Robust exception handling with descriptive error messages
- **Pattern Matching**: Regex-based message validation for complex format checking

#### Demo Scenarios:

1. **Basic Buffer Assertions**
   - Size validation (empty, non-empty, specific size)
   - Buffer lifecycle testing (clear, refill, validate)
   - Simple test setup using TestScope helper

2. **Message Content Validation**
   - Text search within log messages
   - First and last message specific assertions
   - Negative assertions (ensuring content is NOT present)

3. **Log Level Analysis**
   - Counting messages by log level
   - Level-specific content validation
   - Multi-level logging scenarios

4. **Pattern Matching and Regex**
   - Complex message format validation
   - User ID, temperature, file size pattern matching
   - Exception stack trace pattern recognition

5. **Custom Predicate Assertions**
   - Message length analysis (short, medium, long messages)
   - Custom validation logic with lambda functions
   - Any-match and all-match scenarios

6. **Chronological Order Validation**
   - Timestamp ordering verification
   - Multi-threaded logging scenario validation
   - Time-sensitive logging analysis

7. **Performance Testing**
   - High-volume message logging (500+ messages)
   - Assertion performance measurement
   - Buffer operation benchmarking

8. **Error Handling and Edge Cases**
   - Assertion failure testing
   - Exception message validation
   - Robust error reporting

9. **Comprehensive Real-World Scenario**
   - Application startup/shutdown simulation
   - Multi-level logging with error conditions
   - Complete application lifecycle validation
   - Pattern-based validation for user operations

#### Extensions Included:
- **buffer_assertions.h**: Core assertion utilities
- **test_helpers.h**: Reusable test setup and performance measurement classes

#### Use Cases:
- **Unit Testing**: Comprehensive log validation in automated tests
- **Integration Testing**: Multi-component logging scenario validation
- **Performance Analysis**: Logging system performance characterization
- **Debugging Tools**: Interactive log content analysis and validation
- **Quality Assurance**: Systematic logging behavior verification

## Building and Running

### Build the demos:
```bash
mkdir build && cd build
cmake ..
make demo_file_observer         # For the file observer demo
make demo_log_level_filtering   # For the log level filtering demo
make demo_custom_formatting     # For the custom types demo
make demo_container_formatting  # For the container formatting demo
make demo_auto_flushing         # For the auto-flushing scope demo
make demo_debug_scope           # For the debug scope RAII demo
make demo_cerr_observer         # For the cerr observer demo
make demo_exception_formatting  # For the exception formatting demo
make demo_buffer_assertions     # For the buffer assertions extension demo
make ulog_demo                  # For the main demo
```

### Run the demos:
```bash
# Run the file observer demo
./demo_file_observer

# Run the log level filtering demo
./demo_log_level_filtering

# Run the custom types demo
./demo_custom_formatting

# Run the container formatting demo
./demo_container_formatting

# Run the auto-flushing scope demo
./demo_auto_flushing

# Run the debug scope RAII demo
./demo_debug_scope

# Run the cerr observer demo
./demo_cerr_observer

# Run the exception formatting demo
./demo_exception_formatting

# Run the buffer assertions extension demo
./demo_buffer_assertions

# Run the main demo
./ulog_demo
```

### Expected Output Files

After running `demo_file_observer`, the following files will be created:

1. `demo_basic.log` - Basic file logging demo output
2. `demo_raii.log` - RAII observer management demo output  
3. `demo_general.log` - General logging (all message levels)
4. `demo_errors.log` - Copy of general logging 
5. `demo_errors_only.log` - Filtered logging (ERROR and FATAL only)

## Implementation Details

### FileObserver Class

The `FileObserver` class implements the `ulog::LogObserver` interface:

```cpp
class FileObserver : public ulog::LogObserver {
public:
    explicit FileObserver(const std::string& filename, bool append = true);
    
    void handleRegistered(const std::string& logger_name) override;
    void handleUnregistered(const std::string& logger_name) override;
    void handleNewMessage(const ulog::LogEntry& entry) override;
    void handleFlush(const std::string& logger_name) override;
    
    // Additional utility methods
    const std::string& getFilename() const;
    bool isOpen() const;
};
```

### Key Features:

- **Thread Safety**: Uses `std::mutex` to protect file operations
- **RAII**: Automatically opens file in constructor, closes in destructor
- **Error Handling**: Throws exceptions for file operation failures
- **Flexible Modes**: Supports both append and overwrite file modes
- **Immediate Flush**: Calls `flush()` after each write for data persistence

### Usage Example:

```cpp
// Create logger
auto& logger = ulog::getLogger("MyApp");

// Create file observer
auto fileObserver = std::make_shared<FileObserver>("app.log", true); // append mode

// Option 1: Manual management
logger.add_observer(fileObserver);
logger.info("This will be written to app.log");
logger.remove_observer(fileObserver);

// Option 2: RAII management
{
    ulog::ObserverScope scope(logger, fileObserver);
    logger.info("This will also be written to app.log");
} // Observer automatically removed here
```

This demo fulfills the first todo item by providing a complete, production-ready implementation of file output via the observer pattern.
