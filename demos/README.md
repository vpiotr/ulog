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

### 5. demo_debug_scope.cpp
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

## Building and Running

### Build the demos:
```bash
mkdir build && cd build
cmake ..
make demo_file_observer      # For the file observer demo
make demo_log_level_filtering # For the log level filtering demo
make demo_custom_formatting  # For the custom types demo
make demo_auto_flushing      # For the auto-flushing scope demo
make demo_debug_scope        # For the debug scope RAII demo
make ulog_demo              # For the main demo
```

### Run the demos:
```bash
# Run the file observer demo
./demo_file_observer

# Run the log level filtering demo
./demo_log_level_filtering

# Run the custom types demo
./demo_custom_formatting

# Run the auto-flushing scope demo
./demo_auto_flushing

# Run the debug scope RAII demo
./demo_debug_scope

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
