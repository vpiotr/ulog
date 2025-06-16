#ifndef ULOG_THREAD_ID_OBSERVER_H
#define ULOG_THREAD_ID_OBSERVER_H

/**
 * @file thread_id_observer.h
 * @brief Thread-safe observer wrapper that adds thread ID prefix to messages
 * @author ulog team
 * @version 1.0.0
 * 
 * This extension provides a thread-safe observer wrapper that automatically
 * prefixes all log messages with the current thread ID in the format
 * [tid:<value>] before forwarding them to the target observer.
 */

#include "ulog/ulog.h"
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

namespace ulog {
namespace extensions {

/**
 * @brief Thread-safe observer wrapper that adds thread ID prefix to messages
 * 
 * This class wraps around another observer and automatically prefixes all
 * log messages with the thread ID in the format [tid:<value>] before
 * forwarding them to the target observer.
 * 
 * @code{.cpp}
 * // Create file observer for output
 * auto file_observer = std::make_unique<FileObserver>("app.log");
 * 
 * // Wrap with thread ID observer
 * auto thread_observer = std::make_unique<ThreadIdObserver>(std::move(file_observer));
 * 
 * // Register with logger
 * logger.register_observer(std::move(thread_observer));
 * @endcode
 */
class ThreadIdObserver : public ulog::LogObserver {
private:
    std::unique_ptr<ulog::LogObserver> target_observer_;
    mutable std::mutex mutex_;
    
public:
    /**
     * @brief Constructor
     * @param target Observer to forward messages to after adding thread prefix
     * @throws std::invalid_argument if target is nullptr
     */
    explicit ThreadIdObserver(std::unique_ptr<ulog::LogObserver> target)
        : target_observer_(std::move(target)) {
        if (!target_observer_) {
            throw std::invalid_argument("Target observer cannot be null");
        }
    }
    
    /**
     * @brief Called when observer is registered
     * @param logger_name Name of the logger
     */
    void handleRegistered(const std::string& logger_name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        target_observer_->handleRegistered(logger_name);
    }
    
    /**
     * @brief Called when observer is unregistered
     * @param logger_name Name of the logger
     */
    void handleUnregistered(const std::string& logger_name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        target_observer_->handleUnregistered(logger_name);
    }
    
    /**
     * @brief Called when new log message is created
     * @param entry Log entry with message modified to include thread ID
     */
    void handleNewMessage(const LogEntry& entry) override {
        // Create modified entry with thread ID prefix
        LogEntry modified_entry(
            entry.timestamp,
            entry.level,
            entry.logger_name,
            addThreadIdPrefix(entry.message)
        );
        
        std::lock_guard<std::mutex> lock(mutex_);
        target_observer_->handleNewMessage(modified_entry);
    }
    
    /**
     * @brief Called when logger is flushed
     * @param logger_name Name of the logger
     */
    void handleFlush(const std::string& logger_name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        target_observer_->handleFlush(logger_name);
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

#endif // ULOG_THREAD_ID_OBSERVER_H
