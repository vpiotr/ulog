/**
 * @file demo_log_level_filtering.cpp
 * @brief Demonstration of log level filtering functionality
 * 
 * This demo shows how to use the log level filtering feature to control
 * which messages are logged based on their severity level.
 */

#include "ulog/ulog.h"
#include <iostream>
#include <unordered_map>

/**
 * @brief Demo function showcasing log level filtering
 */
void demo_log_level_filtering() {
    std::cout << "\n=== DEMO: Log Level Filtering ===" << std::endl;
    
    auto& logger = ulog::getLogger("FilterDemo");
    
    std::cout << "\n1. Default behavior (INFO level - filters out TRACE and DEBUG):" << std::endl;
    std::cout << "Current log level: " << ulog::to_string(logger.get_log_level()) << std::endl;
    
    logger.trace("This TRACE message will NOT appear (filtered)");
    logger.debug("This DEBUG message will NOT appear (filtered)");
    logger.info("This is an INFO message");
    logger.warn("This is a WARN message");
    logger.error("This is an ERROR message");
    logger.fatal("This is a FATAL message");
    
    std::cout << "\n2. Set log level to TRACE (all messages logged):" << std::endl;
    logger.set_log_level(ulog::LogLevel::TRACE);
    std::cout << "Current log level: " << ulog::to_string(logger.get_log_level()) << std::endl;
    
    logger.trace("This TRACE message WILL appear");
    logger.debug("This DEBUG message WILL appear");
    logger.info("This INFO message WILL appear");
    logger.warn("This WARN message WILL appear");
    logger.error("This ERROR message WILL appear");
    logger.fatal("This FATAL message WILL appear");
    
    std::cout << "\n3. Set log level to INFO (filters out TRACE and DEBUG):" << std::endl;
    logger.set_log_level(ulog::LogLevel::INFO);
    std::cout << "Current log level: " << ulog::to_string(logger.get_log_level()) << std::endl;
    
    logger.trace("This TRACE message will NOT appear");
    logger.debug("This DEBUG message will NOT appear");
    logger.info("This INFO message WILL appear");
    logger.warn("This WARN message WILL appear");
    logger.error("This ERROR message WILL appear");
    logger.fatal("This FATAL message WILL appear");
    
    std::cout << "\n3. Set log level to ERROR (only ERROR and FATAL logged):" << std::endl;
    logger.set_log_level(ulog::LogLevel::ERROR);
    std::cout << "Current log level: " << ulog::to_string(logger.get_log_level()) << std::endl;
    
    logger.trace("This TRACE message will NOT appear");
    logger.debug("This DEBUG message will NOT appear");
    logger.info("This INFO message will NOT appear");
    logger.warn("This WARN message will NOT appear");
    logger.error("This ERROR message WILL appear");
    logger.fatal("This FATAL message WILL appear");
    
    std::cout << "\n4. Set log level to OFF (no messages logged):" << std::endl;
    logger.set_log_level(ulog::LogLevel::OFF);
    std::cout << "Current log level: " << ulog::to_string(logger.get_log_level()) << std::endl;
    
    logger.trace("This TRACE message will NOT appear");
    logger.debug("This DEBUG message will NOT appear");
    logger.info("This INFO message will NOT appear");
    logger.warn("This WARN message will NOT appear");
    logger.error("This ERROR message will NOT appear");
    logger.fatal("This FATAL message will NOT appear");
    
    std::cout << "No messages should have appeared above (all filtered out)." << std::endl;
    
    // Reset to default for any subsequent demos
    logger.set_log_level(ulog::LogLevel::INFO);
    std::cout << "\n5. Reset to INFO level (back to default behavior):" << std::endl;
    logger.info("Logger reset to INFO level - default behavior restored");
}

/**
 * @brief Demo function showcasing log level filtering with memory buffer
 */
void demo_log_level_filtering_with_buffer() {
    std::cout << "\n=== DEMO: Log Level Filtering with Memory Buffer ===" << std::endl;
    
    auto& logger = ulog::getLogger("FilterBufferDemo");
    
    // Enable buffer
    logger.enable_buffer(10);
    
    // Set to WARN level
    logger.set_log_level(ulog::LogLevel::WARN);
    std::cout << "Enabled buffer and set log level to WARN" << std::endl;
    std::cout << "Sending messages at all levels..." << std::endl;
    
    logger.trace("TRACE: This will be filtered out");
    logger.debug("DEBUG: This will be filtered out");
    logger.info("INFO: This will be filtered out");
    logger.warn("WARN: This will be stored in buffer");
    logger.error("ERROR: This will be stored in buffer");
    logger.fatal("FATAL: This will be stored in buffer");
    
    auto buffer = logger.buffer();
    if (buffer) {
        std::cout << "\nBuffer contains " << buffer->size() << " messages:" << std::endl;
        for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
            std::cout << "  " << it->formatted_message() << std::endl;
        }
    }
    
    logger.disable_buffer();
    logger.set_log_level(ulog::LogLevel::INFO);  // Reset to default
}

/**
 * @brief Demo function showcasing log level filtering with observer
 */
void demo_log_level_filtering_with_observer() {
    std::cout << "\n=== DEMO: Log Level Filtering with Observer ===" << std::endl;
    
    // Custom observer that counts messages by level
    class FilteringObserver : public ulog::LogObserver {
    public:
        void handleNewMessage(const ulog::LogEntry& entry) override {
            counts_[entry.level]++;
            std::cout << "  [OBSERVER] Received: " << ulog::to_string(entry.level) 
                      << " - " << entry.message << std::endl;
        }
        
        void printStats() const {
            std::cout << "\nObserver Statistics:" << std::endl;
            for (const auto& [level, count] : counts_) {
                if (count > 0) {
                    std::cout << "  " << ulog::to_string(level) << ": " << count << " messages" << std::endl;
                }
            }
        }
        
    private:
        std::unordered_map<ulog::LogLevel, int> counts_;
    };
    
    auto& logger = ulog::getLogger("FilterObserverDemo");
    auto observer = std::make_shared<FilteringObserver>();
    
    logger.add_observer(observer);
    
    // Set to INFO level
    logger.set_log_level(ulog::LogLevel::INFO);
    std::cout << "Added observer and set log level to INFO" << std::endl;
    std::cout << "Sending messages at all levels:" << std::endl;
    
    logger.trace("TRACE: Observer will NOT receive this");
    logger.debug("DEBUG: Observer will NOT receive this");
    logger.info("INFO: Observer will receive this");
    logger.warn("WARN: Observer will receive this");
    logger.error("ERROR: Observer will receive this");
    logger.fatal("FATAL: Observer will receive this");
    
    observer->printStats();
    
    logger.remove_observer(observer);
    logger.set_log_level(ulog::LogLevel::INFO);  // Reset to default
}

/**
 * @brief Main function
 */
int main() {
    std::cout << "ulog - Log Level Filtering Demo" << std::endl;
    std::cout << "===============================" << std::endl;
    
    try {
        demo_log_level_filtering();
        demo_log_level_filtering_with_buffer();
        demo_log_level_filtering_with_observer();
        
        std::cout << "\n=== Demo completed successfully! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
