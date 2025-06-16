#ifndef ULOG_THREAD_AWARE_LOGGER_H
#define ULOG_THREAD_AWARE_LOGGER_H

/**
 * @file thread_aware_logger.h
 * @brief Thread-aware logger wrapper that adds thread ID to all messages
 * @author ulog team
 * @version 1.0.0
 * 
 * This extension provides a logger wrapper that automatically adds thread IDs
 * to all log messages before they are processed, ensuring that both buffer
 * storage and observers receive messages with thread identification.
 */

#include "ulog/ulog.h"
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

namespace ulog {
namespace extensions {

/**
 * @brief Thread-aware logger wrapper that adds thread ID to all messages
 * 
 * This class wraps around a standard logger and automatically prefixes all
 * log messages with the thread ID in the format [tid:<value>] before
 * forwarding them to the underlying logger. This ensures that both the
 * buffer and observers receive messages with thread identification.
 * 
 * @code{.cpp}
 * auto& base_logger = ulog::getLogger("app");
 * base_logger.enable_buffer(1000);
 * 
 * ThreadAwareLogger thread_logger(base_logger);
 * thread_logger.info("This message will have thread ID");
 * @endcode
 */
class ThreadAwareLogger {
private:
    ulog::Logger& base_logger_;
    mutable std::mutex mutex_;
    
public:
    /**
     * @brief Constructor
     * @param base_logger Reference to the underlying logger
     */
    explicit ThreadAwareLogger(ulog::Logger& base_logger)
        : base_logger_(base_logger) {
    }
    
    /**
     * @brief Get reference to underlying logger
     * @return Reference to base logger
     */
    ulog::Logger& base_logger() {
        return base_logger_;
    }
    
    /**
     * @brief Get const reference to underlying logger
     * @return Const reference to base logger
     */
    const ulog::Logger& base_logger() const {
        return base_logger_;
    }
    
    // Logging methods with automatic thread ID prefixing
    
    /**
     * @brief Log trace message with thread ID
     */
    template<typename... Args>
    void trace(const std::string& message, Args&&... args) {
        log(LogLevel::TRACE, message, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Log debug message with thread ID
     */
    template<typename... Args>
    void debug(const std::string& message, Args&&... args) {
        log(LogLevel::DEBUG, message, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Log info message with thread ID
     */
    template<typename... Args>
    void info(const std::string& message, Args&&... args) {
        log(LogLevel::INFO, message, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Log warning message with thread ID
     */
    template<typename... Args>
    void warn(const std::string& message, Args&&... args) {
        log(LogLevel::WARN, message, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Log error message with thread ID
     */
    template<typename... Args>
    void error(const std::string& message, Args&&... args) {
        log(LogLevel::ERROR, message, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Log fatal message with thread ID
     */
    template<typename... Args>
    void fatal(const std::string& message, Args&&... args) {
        log(LogLevel::FATAL, message, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Generic log method with thread ID
     */
    template<typename... Args>
    void log(LogLevel level, const std::string& message, Args&&... args) {
        std::string thread_prefixed_message = addThreadIdPrefix(message);
        
        // Use the appropriate public method based on log level
        switch (level) {
            case LogLevel::TRACE:
                base_logger_.trace(thread_prefixed_message, std::forward<Args>(args)...);
                break;
            case LogLevel::DEBUG:
                base_logger_.debug(thread_prefixed_message, std::forward<Args>(args)...);
                break;
            case LogLevel::INFO:
                base_logger_.info(thread_prefixed_message, std::forward<Args>(args)...);
                break;
            case LogLevel::WARN:
                base_logger_.warn(thread_prefixed_message, std::forward<Args>(args)...);
                break;
            case LogLevel::ERROR:
                base_logger_.error(thread_prefixed_message, std::forward<Args>(args)...);
                break;
            case LogLevel::FATAL:
                base_logger_.fatal(thread_prefixed_message, std::forward<Args>(args)...);
                break;
            case LogLevel::OFF:
                // Do nothing for OFF level
                break;
        }
    }
    
    // Delegate methods for logger configuration
    
    /**
     * @brief Enable buffer with specified size
     */
    void enable_buffer(size_t size) {
        base_logger_.enable_buffer(size);
    }
    
    /**
     * @brief Disable buffer
     */
    void disable_buffer() {
        base_logger_.disable_buffer();
    }
    
    /**
     * @brief Get buffer reference
     */
    const LogBuffer* buffer() {
        return base_logger_.buffer();
    }
    
    /**
     * @brief Get const buffer reference
     */
    const LogBuffer* buffer() const {
        return base_logger_.buffer();
    }
    
    /**
     * @brief Set log level
     */
    void set_log_level(LogLevel level) {
        base_logger_.set_log_level(level);
    }
    
    /**
     * @brief Get current log level
     */
    LogLevel get_log_level() const {
        return base_logger_.get_log_level();
    }
    
    /**
     * @brief Add observer
     */
    void add_observer(std::shared_ptr<LogObserver> observer) {
        base_logger_.add_observer(observer);
    }
    
    /**
     * @brief Remove observer
     */
    void remove_observer(std::shared_ptr<LogObserver> observer) {
        base_logger_.remove_observer(observer);
    }
    
    /**
     * @brief Flush logger
     */
    void flush() {
        base_logger_.flush();
    }
    
    /**
     * @brief Get logger name
     */
    const std::string& name() const {
        return base_logger_.name();
    }
    
private:
    /**
     * @brief Get current thread ID as string
     * @return Thread ID formatted as string
     */
    std::string getCurrentThreadId() const {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        return oss.str();
    }
    
    /**
     * @brief Add thread ID prefix to message
     * @param original_message Original log message
     * @return Message with thread ID prefix in format [tid:<value>] <message>
     */
    std::string addThreadIdPrefix(const std::string& original_message) const {
        return "[tid:" + getCurrentThreadId() + "] " + original_message;
    }
};

} // namespace extensions
} // namespace ulog

#endif // ULOG_THREAD_AWARE_LOGGER_H
