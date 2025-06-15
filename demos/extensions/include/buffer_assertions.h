#ifndef ULOG_BUFFER_ASSERTIONS_H
#define ULOG_BUFFER_ASSERTIONS_H

/**
 * @file buffer_assertions.h
 * @brief Buffer assertion extensions for ulog testing
 * @author ulog team
 * @version 1.0.0
 * 
 * This extension provides a collection of assertion utilities for testing
 * log buffer contents in unit tests and demos. It allows you to write
 * log messages and then execute various assertions on the buffer contents.
 */

#include "ulog/ulog.h"
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <regex>

namespace ulog {
namespace extensions {

/**
 * @brief Exception thrown when buffer assertion fails
 */
class BufferAssertionException : public std::runtime_error {
public:
    explicit BufferAssertionException(const std::string& message) 
        : std::runtime_error("Buffer assertion failed: " + message) {}
};

/**
 * @brief Buffer assertion utility class for testing log buffer contents
 * 
 * This class provides various assertion methods to verify the contents
 * of a logger's buffer. It's designed to be used in tests and demos
 * to validate that the correct log messages were written.
 * 
 * @code{.cpp}
 * auto& logger = ulog::getLogger("test");
 * logger.enable_buffer(10);
 * 
 * // Write some messages
 * logger.info("Hello World");
 * logger.error("Error occurred");
 * 
 * // Create assertions and test
 * BufferAssertions assertions(logger);
 * assertions.assert_size(2);
 * assertions.assert_contains_message("Hello World");
 * assertions.assert_level_count(ulog::LogLevel::ERROR, 1);
 * @endcode
 */
class BufferAssertions {
public:
    /**
     * @brief Constructor
     * @param logger Reference to the logger to test
     * @throws std::runtime_error if logger doesn't have buffer enabled
     */
    explicit BufferAssertions(ulog::Logger& logger) : logger_(logger) {
        if (!logger_.buffer()) {
            throw std::runtime_error("Logger must have buffer enabled for assertions");
        }
    }
    
    /**
     * @brief Assert that buffer has specific size
     * @param expected_size Expected number of entries in buffer
     * @throws BufferAssertionException if assertion fails
     */
    void assert_size(size_t expected_size) const {
        auto buffer = logger_.buffer();
        size_t actual_size = buffer->size();
        if (actual_size != expected_size) {
            throw BufferAssertionException(
                "Expected buffer size " + std::to_string(expected_size) + 
                ", but got " + std::to_string(actual_size)
            );
        }
    }
    
    /**
     * @brief Assert that buffer is empty
     * @throws BufferAssertionException if assertion fails
     */
    void assert_empty() const {
        auto buffer = logger_.buffer();
        if (!buffer->empty()) {
            throw BufferAssertionException(
                "Expected empty buffer, but it contains " + std::to_string(buffer->size()) + " entries"
            );
        }
    }
    
    /**
     * @brief Assert that buffer is not empty
     * @throws BufferAssertionException if assertion fails
     */
    void assert_not_empty() const {
        auto buffer = logger_.buffer();
        if (buffer->empty()) {
            throw BufferAssertionException("Expected non-empty buffer, but it is empty");
        }
    }
    
    /**
     * @brief Assert that buffer contains a message with specific text
     * @param message_text Text to search for in messages
     * @throws BufferAssertionException if assertion fails
     */
    void assert_contains_message(const std::string& message_text) const {
        auto buffer = logger_.buffer();
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            if (it->message.find(message_text) != std::string::npos) {
                return; // Found the message
            }
        }
        throw BufferAssertionException(
            "Expected to find message containing '" + message_text + "' in buffer"
        );
    }
    
    /**
     * @brief Assert that buffer does not contain a message with specific text
     * @param message_text Text to search for in messages
     * @throws BufferAssertionException if assertion fails
     */
    void assert_not_contains_message(const std::string& message_text) const {
        auto buffer = logger_.buffer();
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            if (it->message.find(message_text) != std::string::npos) {
                throw BufferAssertionException(
                    "Expected not to find message containing '" + message_text + "' in buffer, but found it"
                );
            }
        }
    }
    
    /**
     * @brief Assert that buffer contains a message matching regex pattern
     * @param pattern Regex pattern to match against messages
     * @throws BufferAssertionException if assertion fails
     */
    void assert_contains_pattern(const std::string& pattern) const {
        auto buffer = logger_.buffer();
        std::regex regex_pattern(pattern);
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            if (std::regex_search(it->message, regex_pattern)) {
                return; // Found matching message
            }
        }
        throw BufferAssertionException(
            "Expected to find message matching pattern '" + pattern + "' in buffer"
        );
    }
    
    /**
     * @brief Assert that buffer contains specific number of entries with given log level
     * @param level Log level to count
     * @param expected_count Expected number of entries with this level
     * @throws BufferAssertionException if assertion fails
     */
    void assert_level_count(ulog::LogLevel level, size_t expected_count) const {
        auto buffer = logger_.buffer();
        size_t actual_count = 0;
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            if (it->level == level) {
                actual_count++;
            }
        }
        if (actual_count != expected_count) {
            throw BufferAssertionException(
                "Expected " + std::to_string(expected_count) + " entries with level " + 
                to_string(level) + ", but got " + std::to_string(actual_count)
            );
        }
    }
    
    /**
     * @brief Assert that buffer entries are in chronological order
     * @throws BufferAssertionException if assertion fails
     */
    void assert_chronological_order() const {
        auto buffer = logger_.buffer();
        if (buffer->size() <= 1) {
            return; // Empty or single entry buffer is always ordered
        }
        
        auto prev_it = buffer->cbegin();
        for (auto it = buffer->cbegin() + 1; it != buffer->cend(); ++it) {
            if (it->timestamp < prev_it->timestamp) {
                throw BufferAssertionException(
                    "Buffer entries are not in chronological order"
                );
            }
            prev_it = it;
        }
    }
    
    /**
     * @brief Assert that first entry in buffer contains specific message
     * @param message_text Text to search for in first message
     * @throws BufferAssertionException if assertion fails
     */
    void assert_first_message_contains(const std::string& message_text) const {
        auto buffer = logger_.buffer();
        if (buffer->empty()) {
            throw BufferAssertionException("Cannot check first message - buffer is empty");
        }
        
        auto first_entry = buffer->cbegin();
        if (first_entry->message.find(message_text) == std::string::npos) {
            throw BufferAssertionException(
                "Expected first message to contain '" + message_text + 
                "', but got '" + first_entry->message + "'"
            );
        }
    }
    
    /**
     * @brief Assert that last entry in buffer contains specific message
     * @param message_text Text to search for in last message
     * @throws BufferAssertionException if assertion fails
     */
    void assert_last_message_contains(const std::string& message_text) const {
        auto buffer = logger_.buffer();
        if (buffer->empty()) {
            throw BufferAssertionException("Cannot check last message - buffer is empty");
        }
        
        auto last_entry = buffer->cend() - 1;
        if (last_entry->message.find(message_text) == std::string::npos) {
            throw BufferAssertionException(
                "Expected last message to contain '" + message_text + 
                "', but got '" + last_entry->message + "'"
            );
        }
    }
    
    /**
     * @brief Assert that all entries in buffer match a custom predicate
     * @param predicate Function that takes LogEntry and returns bool
     * @param description Description of what the predicate checks (for error message)
     * @throws BufferAssertionException if assertion fails
     */
    void assert_all_match(const std::function<bool(const ulog::LogEntry&)>& predicate, 
                         const std::string& description) const {
        auto buffer = logger_.buffer();
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            if (!predicate(*it)) {
                throw BufferAssertionException(
                    "Not all entries match predicate: " + description
                );
            }
        }
    }
    
    /**
     * @brief Assert that at least one entry in buffer matches a custom predicate
     * @param predicate Function that takes LogEntry and returns bool
     * @param description Description of what the predicate checks (for error message)
     * @throws BufferAssertionException if assertion fails
     */
    void assert_any_match(const std::function<bool(const ulog::LogEntry&)>& predicate,
                         const std::string& description) const {
        auto buffer = logger_.buffer();
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            if (predicate(*it)) {
                return; // Found matching entry
            }
        }
        throw BufferAssertionException(
            "No entries match predicate: " + description
        );
    }
    
    /**
     * @brief Print all buffer entries to console (for debugging)
     * @param prefix Optional prefix for each line
     */
    void print_buffer_contents(const std::string& prefix = "[BUFFER] ") const {
        auto buffer = logger_.buffer();
        std::cout << prefix << "Buffer contains " << buffer->size() << " entries:" << std::endl;
        size_t index = 0;
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            std::cout << prefix << "[" << index++ << "] " << 
                to_string(it->level) << ": " << it->message << std::endl;
        }
    }
    
    /**
     * @brief Get current buffer size
     * @return Number of entries in buffer
     */
    size_t get_buffer_size() const {
        return logger_.buffer()->size();
    }
    
    /**
     * @brief Clear the buffer
     */
    void clear_buffer() {
        logger_.clear_buffer();
    }

private:
    ulog::Logger& logger_;
};

} // namespace extensions
} // namespace ulog

#endif // ULOG_BUFFER_ASSERTIONS_H
