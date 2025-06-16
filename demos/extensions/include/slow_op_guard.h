#ifndef ULOG_SLOW_OP_GUARD_H
#define ULOG_SLOW_OP_GUARD_H

/**
 * @file slow_op_guard.h
 * @brief SlowOpGuard RAII extension for ulog
 * @author ulog team
 * @version 1.0.0
 * 
 * This extension provides SlowOpGuard RAII functionality for automatically
 * logging slow operations. The SlowOpGuard class monitors operation duration
 * and logs a warning message if the operation takes longer than a specified
 * time limit.
 */

#include "ulog/ulog.h"
#include <chrono>
#include <functional>
#include <string>

namespace ulog {
namespace extensions {

/**
 * @brief RAII guard that logs slow operations
 * 
 * This class monitors the duration of an operation and automatically logs
 * a message if the operation takes longer than the specified time limit.
 * The guard starts timing when constructed and checks the elapsed time
 * when destroyed. If the elapsed time exceeds the threshold, it logs
 * a message with the specified log level.
 * 
 * @code{.cpp}
 * auto& logger = ulog::getLogger("demo");
 * 
 * // String message version
 * {
 *     SlowOpGuard guard(logger, std::chrono::milliseconds(100), 
 *                      ulog::LogLevel::WARN, "slow database query");
 *     // perform database operation
 * } // if operation took > 100ms, logs: "WARN: 150 ms - slow database query"
 * 
 * // Lambda message version
 * {
 *     SlowOpGuard guard(logger, std::chrono::milliseconds(50), 
 *                      ulog::LogLevel::ERROR, 
 *                      [](auto elapsed) { 
 *                          return "critical operation took " + std::to_string(elapsed.count()) + "ms"; 
 *                      });
 *     // perform critical operation
 * } // if operation took > 50ms, logs custom message
 * @endcode
 */
class SlowOpGuard {
public:
    using ClockType = std::chrono::steady_clock;
    using TimePoint = ClockType::time_point;
    using Duration = std::chrono::milliseconds;
    using MessageSupplier = std::function<std::string(Duration)>;

private:
    ulog::Logger& logger_;
    Duration time_limit_;
    ulog::LogLevel log_level_;
    TimePoint start_time_;
    std::string static_message_;
    MessageSupplier message_supplier_;
    bool use_supplier_;

public:
    /**
     * @brief Constructor with static message
     * @param logger Reference to the logger instance
     * @param time_limit Maximum allowed operation duration
     * @param log_level Log level to use for slow operation messages
     * @param message Static message to log (will be prefixed with elapsed time)
     */
    SlowOpGuard(ulog::Logger& logger, 
                Duration time_limit,
                ulog::LogLevel log_level, 
                const std::string& message)
        : logger_(logger)
        , time_limit_(time_limit)
        , log_level_(log_level)
        , start_time_(ClockType::now())
        , static_message_(message)
        , use_supplier_(false) {
    }

    /**
     * @brief Constructor with message supplier (lambda)
     * @param logger Reference to the logger instance
     * @param time_limit Maximum allowed operation duration
     * @param log_level Log level to use for slow operation messages
     * @param supplier Function that generates message based on elapsed time
     */
    SlowOpGuard(ulog::Logger& logger, 
                Duration time_limit,
                ulog::LogLevel log_level, 
                MessageSupplier supplier)
        : logger_(logger)
        , time_limit_(time_limit)
        , log_level_(log_level)
        , start_time_(ClockType::now())
        , message_supplier_(supplier)
        , use_supplier_(true) {
    }

    /**
     * @brief Destructor - checks elapsed time and logs if operation was slow
     */
    ~SlowOpGuard() {
        auto end_time = ClockType::now();
        auto elapsed = std::chrono::duration_cast<Duration>(end_time - start_time_);
        
        if (elapsed >= time_limit_) {
            std::string message;
            
            if (use_supplier_) {
                // Use the message supplier function
                message = message_supplier_(elapsed);
            } else {
                // Format static message with elapsed time prefix
                message = std::to_string(elapsed.count()) + " ms - " + static_message_;
            }
            
            // Log the message at the specified level
            switch (log_level_) {
                case ulog::LogLevel::TRACE:
                    logger_.trace(message);
                    break;
                case ulog::LogLevel::DEBUG:
                    logger_.debug(message);
                    break;
                case ulog::LogLevel::INFO:
                    logger_.info(message);
                    break;
                case ulog::LogLevel::WARN:
                    logger_.warn(message);
                    break;
                case ulog::LogLevel::ERROR:
                    logger_.error(message);
                    break;
                case ulog::LogLevel::FATAL:
                    logger_.fatal(message);
                    break;
                default:
                    // For OFF or unknown levels, don't log
                    break;
            }
        }
    }

    /**
     * @brief Get elapsed time since guard creation
     * @return Duration since guard was created
     */
    Duration elapsed() const {
        auto current_time = ClockType::now();
        return std::chrono::duration_cast<Duration>(current_time - start_time_);
    }

    /**
     * @brief Check if operation is currently slow (elapsed > limit)
     * @return true if current elapsed time exceeds the limit
     */
    bool is_slow() const {
        return elapsed() >= time_limit_;
    }

    /**
     * @brief Get the time limit
     * @return The configured time limit
     */
    Duration get_time_limit() const {
        return time_limit_;
    }
};

} // namespace extensions
} // namespace ulog

#endif // ULOG_SLOW_OP_GUARD_H
