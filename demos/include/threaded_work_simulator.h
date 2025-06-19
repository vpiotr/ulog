#ifndef ULOG_THREADED_WORK_SIMULATOR_H
#define ULOG_THREADED_WORK_SIMULATOR_H

/**
 * @file threaded_work_simulator.h
 * @brief Simple work simulation helpers for multi-threaded demo
 * @author ulog team
 * @version 1.0.0
 * 
 * This header provides simple helper functions to simulate various types
 * of work in multi-threaded scenarios for demonstration purposes.
 */

#include "ulog/ulog.h"
#include "thread_aware_logger.h"
#include <thread>
#include <chrono>
#include <random>
#include <vector>
#include <string>

namespace ulog {
namespace demo {

/**
 * @brief Simple configuration for a worker thread
 */
struct WorkerConfig {
    std::string worker_name;                        ///< Name/type of worker
    int num_operations;                             ///< Number of operations to perform
    std::chrono::milliseconds base_delay;           ///< Base delay between operations
    std::chrono::milliseconds max_jitter;           ///< Maximum random jitter to add
    std::vector<std::string> operation_prefixes;    ///< Prefixes for different operation types
};

/**
 * @brief Simple threaded work simulator
 * 
 * This class provides simple methods to simulate different types of work
 * in separate threads, focusing on clean demo code rather than complex logic.
 */
class ThreadedWorkSimulator {
public:
    /**
     * @brief Simulate database worker operations
     * @param logger Thread-aware logger to write to
     * @param config Worker configuration
     */
    static void simulateDatabaseWork(ulog::extensions::ThreadAwareLogger& logger, const WorkerConfig& config) {
        std::mt19937 rng(std::random_device{}());
        
        logger.info("{0} starting with {1} operations", config.worker_name, config.num_operations);
        
        for (int i = 0; i < config.num_operations; ++i) {
            // Choose random operation prefix
            const auto& prefix = config.operation_prefixes[static_cast<size_t>(i) % config.operation_prefixes.size()];
            
            logger.info("{0} operation {1} starting", prefix, i + 1);
            
            // Simulate work with variable delay
            auto delay = config.base_delay + std::chrono::milliseconds(
                rng() % static_cast<unsigned long>(config.max_jitter.count() + 1));
            std::this_thread::sleep_for(delay);
            
            logger.info("{0} operation {1} completed in {2}ms", prefix, i + 1, delay.count());
        }
        
        logger.info("{0} finished all operations", config.worker_name);
    }
    
    /**
     * @brief Simulate web handler operations
     * @param logger Thread-aware logger to write to
     * @param config Worker configuration
     */
    static void simulateWebWork(ulog::extensions::ThreadAwareLogger& logger, const WorkerConfig& config) {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<> error_dist(1, 20); // 5% error rate
        
        logger.info("{0} starting request handling", config.worker_name);
        
        for (int i = 0; i < config.num_operations; ++i) {
            const auto& prefix = config.operation_prefixes[static_cast<size_t>(i) % config.operation_prefixes.size()];
            
            logger.info("{0} request {1} received", prefix, i + 1);
            
            auto delay = config.base_delay + std::chrono::milliseconds(
                rng() % static_cast<unsigned long>(config.max_jitter.count() + 1));
            std::this_thread::sleep_for(delay);
            
            // Occasionally log an error
            if (error_dist(rng) == 1) {
                logger.error("{0} request {1} failed - timeout", prefix, i + 1);
            } else {
                logger.info("{0} request {1} processed successfully", prefix, i + 1);
            }
        }
        
        logger.info("{0} shutting down", config.worker_name);
    }
    
    /**
     * @brief Simulate background processing operations
     * @param logger Thread-aware logger to write to
     * @param config Worker configuration
     */
    static void simulateBackgroundWork(ulog::extensions::ThreadAwareLogger& logger, const WorkerConfig& config) {
        std::mt19937 rng(std::random_device{}());
        
        logger.info("{0} background processing started", config.worker_name);
        
        for (int i = 0; i < config.num_operations; ++i) {
            const auto& prefix = config.operation_prefixes[static_cast<size_t>(i) % config.operation_prefixes.size()];
            
            logger.debug("{0} processing batch {1}", prefix, i + 1);
            
            auto delay = config.base_delay + std::chrono::milliseconds(
                rng() % static_cast<unsigned long>(config.max_jitter.count() + 1));
            std::this_thread::sleep_for(delay);
            
            logger.debug("{0} batch {1} processed", prefix, i + 1);
        }
        
        logger.info("{0} background processing completed", config.worker_name);
    }
    
    /**
     * @brief Simulate system monitoring operations
     * @param logger Thread-aware logger to write to
     * @param config Worker configuration
     */
    static void simulateMonitoringWork(ulog::extensions::ThreadAwareLogger& logger, const WorkerConfig& config) {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<> alert_dist(1, 10); // 10% alert rate
        
        logger.info("{0} monitoring started", config.worker_name);
        
        for (int i = 0; i < config.num_operations; ++i) {
            const auto& prefix = config.operation_prefixes[static_cast<size_t>(i) % config.operation_prefixes.size()];
            
            logger.trace("{0} checking system health", prefix);
            
            auto delay = config.base_delay + std::chrono::milliseconds(
                rng() % static_cast<unsigned long>(config.max_jitter.count() + 1));
            std::this_thread::sleep_for(delay);
            
            // Occasionally trigger an alert
            if (alert_dist(rng) == 1) {
                logger.warn("{0} ALERT: High resource usage detected", prefix);
            } else {
                logger.trace("{0} system healthy", prefix);
            }
        }
        
        logger.info("{0} monitoring stopped", config.worker_name);
    }
};

} // namespace demo
} // namespace ulog

#endif // ULOG_THREADED_WORK_SIMULATOR_H
