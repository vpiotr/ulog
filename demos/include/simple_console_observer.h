#ifndef ULOG_SIMPLE_CONSOLE_OBSERVER_H
#define ULOG_SIMPLE_CONSOLE_OBSERVER_H

/**
 * @file simple_console_observer.h
 * @brief Simple console observer for demo purposes
 * @author ulog team
 * @version 1.0.0
 */

#include "ulog/ulog.h"
#include <iostream>
#include <mutex>

namespace ulog {
namespace demo {

/**
 * @brief Simple console observer that prints messages to stdout
 */
class SimpleConsoleObserver : public ulog::LogObserver {
private:
    mutable std::mutex mutex_;
    
public:
    void handleNewMessage(const LogEntry& entry) override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << entry.formatted_message() << std::endl;
    }
};

} // namespace demo
} // namespace ulog

#endif // ULOG_SIMPLE_CONSOLE_OBSERVER_H
