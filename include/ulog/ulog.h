#ifndef ULOG_ULOG_H
#define ULOG_ULOG_H

/**
 * @file ulog.h
 * @brief Header-only C++ logging library with console output and optional memory buffer
 * @author ulog team
 * @version 1.0.0
 * 
 * @mainpage ulog - Lightweight C++ Logging Library
 * 
 * @section introduction Introduction
 * 
 * ulog is a lightweight, header-only C++ logging library that provides:
 * - Fast console output (active by default)
 * - Optional memory buffer for log entry storage
 * - Thread-safe logger registry
 * - Observer pattern for log entry handling
 * - Flexible message formatting with anonymous and positional parameters
 * - RAII-based resource management
 * - Optional mutex configuration for performance tuning
 * 
 * @section usage_sec Basic Usage
 * 
 * @code{.cpp}
 * #include "ulog/ulog.h"
 * 
 * // Get global logger
 * auto logger = ulog::getLogger();
 * logger.info("Hello, world!");
 * 
 * // Get named logger
 * auto namedLogger = ulog::getLogger("MyApp");
 * namedLogger.debug("Debug message: {0}", value);
 * 
 * // Enable memory buffer
 * logger.enable_buffer(100);
 * logger.info("Buffered message");
 * logger.flush();
 * @endcode
 */

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <atomic>
#include <iomanip>
#include <ctime>
#include <type_traits>

#ifdef ULOG_USE_USTR
#include "ustr/ustr.h" // Assuming ustr.h is in an include path accessible as "ustr/ustr.h"
#else
namespace ulog {
namespace ustr {
    /**
     * @brief Convert value to string using stringstream
     * @tparam T Type of value to convert
     * @param value Value to convert
     * @return String representation of value
     */
    template<typename T>
    std::string to_string(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
    
    /**
     * @brief Specialization for std::string
     */
    template<>
    inline std::string to_string<std::string>(const std::string& value) {
        return value;
    }
    
    /**
     * @brief Specialization for const char*
     */
    template<>
    inline std::string to_string<const char*>(const char* const& value) {
        return std::string(value);
    }
}
} // namespace ulog
#endif // ULOG_USE_USTR

// Optional mutex configuration
#ifndef ULOG_USE_MUTEX_FOR_CONSOLE
#define ULOG_USE_MUTEX_FOR_CONSOLE 1  ///< Enable mutex for console operations (default: enabled)
#endif

#ifndef ULOG_USE_MUTEX_FOR_BUFFER
#define ULOG_USE_MUTEX_FOR_BUFFER 1   ///< Enable mutex for buffer operations (default: enabled)
#endif

#ifndef ULOG_USE_MUTEX_FOR_OBSERVERS
#define ULOG_USE_MUTEX_FOR_OBSERVERS 1 ///< Enable mutex for observer operations (default: enabled)
#endif

namespace ulog {

/**
 * @brief Log level enumeration
 */
enum class LogLevel {
    OFF = -1,
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

/**
 * @brief Convert log level to string representation
 * @param level Log level to convert
 * @return String representation of log level
 */
inline std::string to_string(LogLevel level) {
    switch (level) {
        case LogLevel::OFF: return "OFF";
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Log entry structure containing all log information
 */
struct LogEntry {
    std::chrono::system_clock::time_point timestamp; ///< When the log entry was created
    LogLevel level;                                   ///< Log level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
    std::string logger_name;                         ///< Name of the logger that created this entry
    std::string message;                             ///< The formatted message with all parameters substituted (e.g., "Hello user tom123")
    
    /**
     * @brief Constructor for log entry
     * @param ts Timestamp
     * @param lvl Log level
     * @param name Logger name
     * @param msg Formatted message content with parameters already substituted
     */
    LogEntry(std::chrono::system_clock::time_point ts, LogLevel lvl, 
             const std::string& name, const std::string& msg)
        : timestamp(ts), level(lvl), logger_name(name), message(msg) {}
    
    /**
     * @brief Get fully formatted log line with timestamp, level, logger name, and message
     * @return Complete formatted log line ready for output (e.g., "2025-06-15 10:30:15.123 [INFO] [MyApp] Hello user tom123")
     * @note This differs from the 'message' field which contains the formatted message with substituted parameters
     *       but without timestamp/level prefix formatting
     */
    std::string formatted_message() const {
        std::ostringstream oss;
        auto time_t = std::chrono::system_clock::to_time_t(timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            timestamp.time_since_epoch()) % 1000;
        
        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));
        
        oss << time_str << "." << std::setfill('0') << std::setw(3) << ms.count();
        oss << " [" << to_string(level) << "]";
        if (!logger_name.empty()) {
            oss << " [" << logger_name << "]";
        }
        oss << " " << message;
        
        return oss.str();
    }
};

/**
 * @brief Abstract observer interface for log events
 */
class LogObserver {
public:
    virtual ~LogObserver() = default;
    
    /**
     * @brief Called when observer is registered
     * @param logger_name Name of the logger
     */
    virtual void handleRegistered(const std::string& logger_name) { (void)logger_name; }
    
    /**
     * @brief Called when observer is unregistered
     * @param logger_name Name of the logger
     */
    virtual void handleUnregistered(const std::string& logger_name) { (void)logger_name; }
    
    /**
     * @brief Called when new log message is created
     * @param entry Log entry
     */
    virtual void handleNewMessage(const LogEntry& entry) = 0;
    
    /**
     * @brief Called when logger is flushed
     * @param logger_name Name of the logger
     */
    virtual void handleFlush(const std::string& logger_name) { (void)logger_name; }
};

/**
 * @brief Buffer for storing log entries in memory
 */
class LogBuffer {
public:
    using iterator = std::vector<LogEntry>::const_iterator;
    
    /**
     * @brief Constructor
     * @param capacity Maximum number of entries (0 = unlimited)
     */
    explicit LogBuffer(size_t capacity = 0) : capacity_(capacity) {}
    
    /**
     * @brief Add log entry to buffer
     * @param entry Log entry to add
     */
    void add(const LogEntry& entry) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (capacity_ > 0 && entries_.size() >= capacity_) {
            entries_.erase(entries_.begin());
        }
        entries_.push_back(entry);
    }
    
    /**
     * @brief Clear all entries from buffer
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_.clear();
    }
    
    /**
     * @brief Get begin iterator
     * @return Begin iterator
     */
    iterator cbegin() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.cbegin();
    }
    
    /**
     * @brief Get end iterator
     * @return End iterator
     */
    iterator cend() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.cend();
    }
    
    /**
     * @brief Get current size
     * @return Number of entries in buffer
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.size();
    }
    
    /**
     * @brief Check if buffer is empty
     * @return True if buffer is empty
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.empty();
    }

private:
    mutable std::mutex mutex_;
    std::vector<LogEntry> entries_;
    size_t capacity_;
};

/**
 * @brief Message formatter for handling anonymous and positional parameters
 */
class MessageFormatter {
public:
    /**
     * @brief Format message with no parameters
     * @param format Format string
     * @return Formatted message
     */
    static std::string format(const std::string& format) {
        return format;
    }
    
    /**
     * @brief Format message with parameters
     * @tparam Args Parameter types
     * @param format Format string
     * @param args Arguments to format
     * @return Formatted message
     */
    template<typename... Args>
    static std::string format(const std::string& format, Args&&... args) {
#ifdef ULOG_USE_USTR
        std::vector<std::string> arg_strings = {::ustr::to_string(args)...};
#else
        std::vector<std::string> arg_strings = {ulog::ustr::to_string(args)...};
#endif // ULOG_USE_USTR
        return format_impl(format, arg_strings);
    }

private:
    static std::string format_impl(const std::string& format, const std::vector<std::string>& args) {
        std::string result = format;
        size_t arg_index = 0;
        
        // Handle anonymous parameters {?} - process sequentially
        size_t pos = 0;
        while ((pos = result.find("{?}", pos)) != std::string::npos && arg_index < args.size()) {
            result.replace(pos, 3, args[arg_index++]);
            pos += args[arg_index - 1].length();
        }
        
        // Handle positional parameters {0}, {1}, etc.
        for (size_t i = 0; i < args.size(); ++i) {
            std::string placeholder = "{" + std::to_string(i) + "}";
            pos = 0;
            while ((pos = result.find(placeholder, pos)) != std::string::npos) {
                result.replace(pos, placeholder.length(), args[i]);
                pos += args[i].length();
            }
        }
        
        return result;
    }
};

/**
 * @brief Main logger class
 */
class Logger {
public:
    /**
     * @brief Constructor
     * @param name Logger name (empty for global logger)
     */
    explicit Logger(const std::string& name = "") 
        : name_(name), console_enabled_(true), buffer_enabled_(false), log_level_(LogLevel::INFO), clean_message_(true) {}
    
    /**
     * @brief Log trace message
     * @tparam Args Parameter types
     * @param format Format string
     * @param args Arguments
     */
    template<typename... Args>
    void trace(const std::string& format, Args&&... args) {
        log(LogLevel::TRACE, format, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Log debug message
     * @tparam Args Parameter types
     * @param format Format string
     * @param args Arguments
     */
    template<typename... Args>
    void debug(const std::string& format, Args&&... args) {
        log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Log info message
     * @tparam Args Parameter types
     * @param format Format string
     * @param args Arguments
     */
    template<typename... Args>
    void info(const std::string& format, Args&&... args) {
        log(LogLevel::INFO, format, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Log warning message
     * @tparam Args Parameter types
     * @param format Format string
     * @param args Arguments
     */
    template<typename... Args>
    void warn(const std::string& format, Args&&... args) {
        log(LogLevel::WARN, format, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Log error message
     * @tparam Args Parameter types
     * @param format Format string
     * @param args Arguments
     */
    template<typename... Args>
    void error(const std::string& format, Args&&... args) {
        log(LogLevel::ERROR, format, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Log fatal message
     * @tparam Args Parameter types
     * @param format Format string
     * @param args Arguments
     */
    template<typename... Args>
    void fatal(const std::string& format, Args&&... args) {
        log(LogLevel::FATAL, format, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Enable memory buffer
     * @param capacity Maximum number of entries (0 = unlimited)
     */
    void enable_buffer(size_t capacity = 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_enabled_ = true;
        buffer_ = std::make_unique<LogBuffer>(capacity);
    }
    
    /**
     * @brief Disable memory buffer
     */
    void disable_buffer() {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_enabled_ = false;
        buffer_.reset();
    }
    
    /**
     * @brief Clear buffer contents
     */
    void clear_buffer() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (buffer_) {
            buffer_->clear();
        }
    }
    
    /**
     * @brief Enable console output
     */
    void enable_console() {
        std::lock_guard<std::mutex> lock(mutex_);
        console_enabled_ = true;
    }
    
    /**
     * @brief Disable console output
     */
    void disable_console() {
        std::lock_guard<std::mutex> lock(mutex_);
        console_enabled_ = false;
    }
    
    /**
     * @brief Flush console output
     */
    void flush() {
        // Flush console with optional mutex
        if (console_enabled_) {
#if ULOG_USE_MUTEX_FOR_CONSOLE
            std::lock_guard<std::mutex> console_lock(console_mutex_);
#endif
            std::cout << std::endl;
        }
        
        // Notify observers with optional mutex
        {
#if ULOG_USE_MUTEX_FOR_OBSERVERS
            std::lock_guard<std::mutex> observers_lock(observers_mutex_);
#endif
            notify_observers_flush();
        }
    }
    
    /**
     * @brief Add observer
     * @param observer Observer to add
     */
    void add_observer(std::shared_ptr<LogObserver> observer) {
#if ULOG_USE_MUTEX_FOR_OBSERVERS
        std::lock_guard<std::mutex> observers_lock(observers_mutex_);
#endif
        observers_.push_back(observer);
        observer->handleRegistered(name_);
    }
    
    /**
     * @brief Remove observer
     * @param observer Observer to remove
     */
    void remove_observer(std::shared_ptr<LogObserver> observer) {
#if ULOG_USE_MUTEX_FOR_OBSERVERS
        std::lock_guard<std::mutex> observers_lock(observers_mutex_);
#endif
        auto it = std::find(observers_.begin(), observers_.end(), observer);
        if (it != observers_.end()) {
            (*it)->handleUnregistered(name_);
            observers_.erase(it);
        }
    }
    
    /**
     * @brief Get buffer (read-only access)
     * @return Pointer to buffer or nullptr if not enabled
     */
    const LogBuffer* buffer() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.get();
    }
    
    /**
     * @brief Set log level filter
     * @param level Minimum log level to process
     */
    void set_log_level(LogLevel level) {
        log_level_ = level;
    }
    
    /**
     * @brief Get current log level filter
     * @return Current log level filter
     */
    LogLevel get_log_level() const {
        return log_level_;
    }
    
    /**
     * @brief Get logger name
     * @return Logger name
     */
    const std::string& name() const { return name_; }
    
    /**
     * @brief Enable message cleaning (replaces control characters with hex codes)
     */
    void enable_clean_message() {
        clean_message_ = true;
    }
    
    /**
     * @brief Disable message cleaning
     */
    void disable_clean_message() {
        clean_message_ = false;
    }
    
    /**
     * @brief Check if message cleaning is enabled
     * @return True if message cleaning is enabled
     */
    bool is_clean_message_enabled() const {
        return clean_message_;
    }

private:
    /**
     * @brief Convert character to hex string representation
     * @param ch Character to convert
     * @return Hex string in format \xXX
     */
    static std::string char_to_hex(unsigned char ch) {
        std::ostringstream oss;
        oss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << static_cast<int>(ch);
        return oss.str();
    }

    /**
     * @brief Clean message by replacing control characters with hex codes or spaces
     * Preserves unicode characters while handling ASCII control characters (<32):
     * - Whitespace characters (space, tab, newline, etc.) are replaced with single space
     * - Other control characters are replaced with hex codes
     * @param message Message to clean
     * @return Cleaned message
     */
    static std::string clean_message(const std::string& message) {
        std::string cleaned;
        cleaned.reserve(message.size() * 2); // Reserve extra space for potential hex codes
        
        for (size_t i = 0; i < message.size(); ++i) {
            unsigned char ch = static_cast<unsigned char>(message[i]);
            
            // Check if this is a multi-byte UTF-8 character
            if (ch >= 0x80) {
                // UTF-8 multi-byte character - preserve it
                if ((ch & 0xE0) == 0xC0) {
                    // 2-byte sequence
                    if (i + 1 < message.size()) {
                        cleaned += message.substr(i, 2);
                        i += 1; // Skip next byte
                    } else {
                        // Invalid UTF-8, replace with hex
                        cleaned += char_to_hex(ch);
                    }
                } else if ((ch & 0xF0) == 0xE0) {
                    // 3-byte sequence
                    if (i + 2 < message.size()) {
                        cleaned += message.substr(i, 3);
                        i += 2; // Skip next 2 bytes
                    } else {
                        // Invalid UTF-8, replace with hex
                        cleaned += char_to_hex(ch);
                    }
                } else if ((ch & 0xF8) == 0xF0) {
                    // 4-byte sequence
                    if (i + 3 < message.size()) {
                        cleaned += message.substr(i, 4);
                        i += 3; // Skip next 3 bytes
                    } else {
                        // Invalid UTF-8, replace with hex
                        cleaned += char_to_hex(ch);
                    }
                } else {
                    // Invalid UTF-8 start byte, replace with hex
                    cleaned += char_to_hex(ch);
                }
            } else if (ch < 32) {
                // ASCII control character - check if it's whitespace
                if (ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f') {
                    // Whitespace characters: replace with single space
                    cleaned += ' ';
                } else {
                    // Other control characters: replace with hex code
                    cleaned += char_to_hex(ch);
                }
            } else {
                // Regular ASCII character (32 and above), keep as-is
                cleaned += ch;
            }
        }
        
        return cleaned;
    }

    template<typename... Args>
    void log(LogLevel level, const std::string& format, Args&&... args) {
        // Check if logging is enabled for this level
        LogLevel current_level = log_level_;
        if (current_level == LogLevel::OFF || level < current_level) {
            return;
        }
        
        auto message = MessageFormatter::format(format, std::forward<Args>(args)...);
        
        // Apply message cleaning if enabled
        if (clean_message_) {
            message = clean_message(message);
        }
        
        auto entry = LogEntry(std::chrono::system_clock::now(), level, name_, message);
        
        // Console output with optional mutex
        if (console_enabled_) {
#if ULOG_USE_MUTEX_FOR_CONSOLE
            std::lock_guard<std::mutex> console_lock(console_mutex_);
#endif
            std::cout << entry.formatted_message() << "\n";
        }
        
        // Buffer operations with optional mutex
        if (buffer_enabled_ && buffer_) {
#if ULOG_USE_MUTEX_FOR_BUFFER
            std::lock_guard<std::mutex> buffer_lock(buffer_mutex_);
#endif
            buffer_->add(entry);
        }
        
        // Observer notifications with optional mutex
        {
#if ULOG_USE_MUTEX_FOR_OBSERVERS
            std::lock_guard<std::mutex> observers_lock(observers_mutex_);
#endif
            notify_observers_message(entry);
        }
    }
    
    void notify_observers_message(const LogEntry& entry) {
        for (auto& observer : observers_) {
            observer->handleNewMessage(entry);
        }
    }
    
    void notify_observers_flush() {
        for (auto& observer : observers_) {
            observer->handleFlush(name_);
        }
    }
    
    std::string name_;
    mutable std::mutex mutex_;
#if ULOG_USE_MUTEX_FOR_CONSOLE
    mutable std::mutex console_mutex_;
#endif
#if ULOG_USE_MUTEX_FOR_BUFFER
    mutable std::mutex buffer_mutex_;
#endif
#if ULOG_USE_MUTEX_FOR_OBSERVERS
    mutable std::mutex observers_mutex_;
#endif
    std::atomic<bool> console_enabled_;
    std::atomic<bool> buffer_enabled_;
    std::atomic<LogLevel> log_level_;
    std::atomic<bool> clean_message_;
    std::unique_ptr<LogBuffer> buffer_;
    std::vector<std::shared_ptr<LogObserver>> observers_;
};

/**
 * @brief RAII observer scope for automatic observer management
 */
class ObserverScope {
public:
    /**
     * @brief Constructor - automatically adds observer
     * @param logger Logger to add observer to
     * @param observer Observer to add
     */
    ObserverScope(Logger& logger, std::shared_ptr<LogObserver> observer)
        : logger_(logger), observer_(observer) {
        logger_.add_observer(observer_);
    }
    
    /**
     * @brief Destructor - automatically removes observer
     */
    ~ObserverScope() {
        logger_.remove_observer(observer_);
    }
    
    // Non-copyable
    ObserverScope(const ObserverScope&) = delete;
    ObserverScope& operator=(const ObserverScope&) = delete;

private:
    Logger& logger_;
    std::shared_ptr<LogObserver> observer_;
};

/**
 * @brief RAII auto-flushing scope for automatic logger flushing
 */
class AutoFlushingScope {
public:
    /**
     * @brief Constructor - stores logger reference
     * @param logger Logger to flush on scope exit
     */
    explicit AutoFlushingScope(Logger& logger)
        : logger_(logger) {
    }
    
    /**
     * @brief Destructor - automatically flushes logger
     */
    ~AutoFlushingScope() {
        logger_.flush();
    }
    
    // Non-copyable, non-movable
    AutoFlushingScope(const AutoFlushingScope&) = delete;
    AutoFlushingScope& operator=(const AutoFlushingScope&) = delete;
    AutoFlushingScope(AutoFlushingScope&&) = delete;
    AutoFlushingScope& operator=(AutoFlushingScope&&) = delete;

private:
    Logger& logger_;
};

/**
 * @brief Logger registry for managing named loggers
 */
class LoggerRegistry {
public:
    /**
     * @brief Get singleton instance
     * @return Registry instance
     */
    static LoggerRegistry& instance() {
        static LoggerRegistry registry;
        return registry;
    }
    
    /**
     * @brief Get or create logger by name
     * @param name Logger name
     * @return Reference to logger
     */
    Logger& get_logger(const std::string& name = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = loggers_.find(name);
        if (it == loggers_.end()) {
            it = loggers_.emplace(name, std::make_unique<Logger>(name)).first;
        }
        return *it->second;
    }
    
    /**
     * @brief Get or create logger using factory
     * @tparam Factory Factory type
     * @param name Logger name
     * @param factory Factory function
     * @return Reference to logger
     */
    template<typename Factory>
    Logger& get_logger(const std::string& name, Factory&& factory) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = loggers_.find(name);
        if (it == loggers_.end()) {
            auto logger = factory(name);
            it = loggers_.emplace(name, std::move(logger)).first;
        }
        return *it->second;
    }

private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<Logger>> loggers_;
};

/**
 * @brief Get global logger
 * @return Reference to global logger
 */
inline Logger& getLogger() {
    return LoggerRegistry::instance().get_logger();
}

/**
 * @brief Get named logger
 * @param name Logger name
 * @return Reference to named logger
 */
inline Logger& getLogger(const std::string& name) {
    return LoggerRegistry::instance().get_logger(name);
}

} // namespace ulog

#endif // ULOG_ULOG_H
