/**
 * @file demo_cerr_observer.cpp
 * @brief Demonstration of logging errors to cerr (stderr) via observer
 * 
 * This demo showcases how to create a custom observer that filters messages
 * and writes error messages to stderr (std::cerr) while allowing other
 * messages to go to normal console output.
 */

#include "ulog/ulog.h"
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @brief Observer that writes error messages (ERROR and FATAL) to stderr
 */
class CerrObserver : public ulog::LogObserver {
public:
    /**
     * @brief Constructor
     * @param name Optional name for the observer for identification
     */
    explicit CerrObserver(const std::string& name = "CerrObserver") : name_(name) {
        std::cout << "[" << name_ << "] Created cerr observer for error messages" << std::endl;
    }
    
    void handleRegistered(const std::string& logger_name) override {
        std::cout << "[" << name_ << "] Observer registered to logger: " << logger_name << std::endl;
    }
    
    void handleUnregistered(const std::string& logger_name) override {
        std::cout << "[" << name_ << "] Observer unregistered from logger: " << logger_name << std::endl;
    }
    
    void handleNewMessage(const ulog::LogEntry& entry) override {
        // Only write ERROR and FATAL messages to stderr
        if (entry.level == ulog::LogLevel::ERROR || entry.level == ulog::LogLevel::FATAL) {
            std::lock_guard<std::mutex> lock(mutex_);
            std::cerr << "[STDERR] " << entry.formatted_message() << std::endl;
            error_count_++;
        }
    }
    
    void handleFlush(const std::string& logger_name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cerr << "[STDERR] # Flush requested for logger: " << logger_name << std::endl;
        std::cerr.flush();
        std::cout << "[" << name_ << "] Flushed stderr for logger: " << logger_name << std::endl;
    }
    
    /**
     * @brief Get the number of error messages processed
     * @return Count of ERROR and FATAL messages
     */
    size_t getErrorCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return error_count_;
    }
    
    /**
     * @brief Get the observer name
     * @return Observer name
     */
    const std::string& getName() const {
        return name_;
    }

private:
    std::string name_;
    size_t error_count_ = 0;
    mutable std::mutex mutex_;
};

/**
 * @brief Demo function showcasing basic cerr observer usage
 */
void demo_basic_cerr_observer() {
    std::cout << "\n=== DEMO: Basic Cerr Observer ===" << std::endl;
    
    auto& logger = ulog::getLogger("CerrDemo");
    
    try {
        // Create cerr observer that writes errors to stderr
        auto cerrObserver = std::make_shared<CerrObserver>("ErrorToStderr");
        
        // Manual observer management
        logger.add_observer(cerrObserver);
        
        std::cout << "Added cerr observer. Console output (INFO, WARN) goes to stdout," << std::endl;
        std::cout << "while ERROR and FATAL messages will go to stderr." << std::endl;
        std::cout << "\nLogging messages at different levels:" << std::endl;
        
        // Log various messages at different levels
        logger.info("Application started - this goes to stdout");
        logger.debug("Debug information - this goes to stdout");
        logger.warn("Warning: configuration file not found - this goes to stdout");
        logger.error("Error: failed to connect to database - this goes to stderr");
        logger.info("Retrying database connection - this goes to stdout");
        logger.fatal("Fatal: critical system failure - this goes to stderr");
        logger.info("System shutdown initiated - this goes to stdout");
        
        // Manually flush to ensure all data is written
        logger.flush();
        
        std::cout << "\nError messages processed by cerr observer: " 
                  << cerrObserver->getErrorCount() << std::endl;
        
        // Remove observer
        logger.remove_observer(cerrObserver);
        
        std::cout << "Demo completed. Error messages were written to stderr." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in demo: " << e.what() << std::endl;
    }
}

/**
 * @brief Demo function showcasing RAII cerr observer management
 */
void demo_raii_cerr_observer() {
    std::cout << "\n=== DEMO: RAII Cerr Observer Management ===" << std::endl;
    
    auto& logger = ulog::getLogger("RAIICerrDemo");
    
    try {
        // Create cerr observer
        auto cerrObserver = std::make_shared<CerrObserver>("RAIIErrorHandler");
        
        std::cout << "Using RAII observer management..." << std::endl;
        
        // RAII observer management
        {
            ulog::ObserverScope scope(logger, cerrObserver);
            
            logger.info("RAII scope started - stdout");
            logger.warn("Processing within RAII scope - stdout");
            logger.error("Error within RAII scope - stderr");
            logger.info("RAII scope ending - stdout");
            
        } // Observer automatically removed here
        
        // This error won't be sent to stderr since observer is removed
        logger.error("Error after RAII scope - goes to normal console");
        
        std::cout << "RAII demo completed. Error messages in scope were written to stderr." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in RAII demo: " << e.what() << std::endl;
    }
}

/**
 * @brief Demo function showcasing multiple observers with cerr
 */
void demo_multiple_observers_with_cerr() {
    std::cout << "\n=== DEMO: Multiple Observers with Cerr ===" << std::endl;
    
    auto& logger = ulog::getLogger("MultiObserverDemo");
    
    try {
        // Create multiple observers
        auto cerrObserver = std::make_shared<CerrObserver>("ErrorHandler");
        
        // Create a custom observer that counts all messages
        class MessageCounterObserver : public ulog::LogObserver {
        public:
            void handleNewMessage(const ulog::LogEntry& entry) override {
                counts_[entry.level]++;
                std::cout << "[COUNTER] Logged: " << ulog::to_string(entry.level) 
                          << " message" << std::endl;
            }
            
            void printStats() const {
                std::cout << "\nMessage Statistics:" << std::endl;
                for (const auto& [level, count] : counts_) {
                    if (count > 0) {
                        std::cout << "  " << ulog::to_string(level) << ": " << count << " messages" << std::endl;
                    }
                }
            }
            
        private:
            std::unordered_map<ulog::LogLevel, int> counts_;
        };
        
        auto counterObserver = std::make_shared<MessageCounterObserver>();
        
        // Add both observers
        logger.add_observer(cerrObserver);
        logger.add_observer(counterObserver);
        
        std::cout << "Added cerr observer and message counter observer." << std::endl;
        std::cout << "Errors will go to stderr, while counter tracks all messages." << std::endl;
        std::cout << "\nLogging various messages:" << std::endl;
        
        // Log messages at different levels
        logger.info("Service initialization started");
        logger.debug("Loading configuration files");
        logger.info("Configuration loaded successfully");
        logger.warn("Deprecated API usage detected");
        logger.error("Failed to validate user credentials");
        logger.info("Attempting fallback authentication");
        logger.error("Fallback authentication failed");
        logger.fatal("Authentication system completely unavailable");
        logger.info("Switching to maintenance mode");
        
        // Flush and show statistics
        logger.flush();
        
        counterObserver->printStats();
        std::cout << "Errors sent to stderr: " << cerrObserver->getErrorCount() << std::endl;
        
        // Cleanup
        logger.remove_observer(cerrObserver);
        logger.remove_observer(counterObserver);
        
        std::cout << "Multiple observers demo completed." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in multiple observers demo: " << e.what() << std::endl;
    }
}

/**
 * @brief Demo function showcasing cerr observer with log level filtering
 */
void demo_cerr_observer_with_filtering() {
    std::cout << "\n=== DEMO: Cerr Observer with Log Level Filtering ===" << std::endl;
    
    auto& logger = ulog::getLogger("FilteringCerrDemo");
    
    try {
        auto cerrObserver = std::make_shared<CerrObserver>("FilteredErrorHandler");
        
        logger.add_observer(cerrObserver);
        
        std::cout << "Testing cerr observer with different log levels..." << std::endl;
        
        // Test with INFO level (ERROR and FATAL should still go to stderr)
        std::cout << "\n1. Log level set to INFO:" << std::endl;
        logger.set_log_level(ulog::LogLevel::INFO);
        
        logger.trace("TRACE: This won't appear anywhere (filtered)");
        logger.debug("DEBUG: This won't appear anywhere (filtered)");
        logger.info("INFO: Application running - stdout");
        logger.warn("WARN: Minor issue detected - stdout");
        logger.error("ERROR: Processing failed - stderr");
        logger.fatal("FATAL: System crash - stderr");
        
        // Test with ERROR level (only ERROR and FATAL appear, both go to stderr)
        std::cout << "\n2. Log level set to ERROR:" << std::endl;
        logger.set_log_level(ulog::LogLevel::ERROR);
        
        logger.trace("TRACE: Filtered out");
        logger.debug("DEBUG: Filtered out");
        logger.info("INFO: Filtered out");
        logger.warn("WARN: Filtered out");
        logger.error("ERROR: Critical error - stderr");
        logger.fatal("FATAL: System failure - stderr");
        
        // Test with OFF level (nothing should appear)
        std::cout << "\n3. Log level set to OFF:" << std::endl;
        logger.set_log_level(ulog::LogLevel::OFF);
        
        logger.error("ERROR: This won't appear (all logging disabled)");
        logger.fatal("FATAL: This won't appear (all logging disabled)");
        
        std::cout << "No messages should have appeared above (all filtered out)." << std::endl;
        
        logger.flush();
        std::cout << "\nTotal errors sent to stderr: " << cerrObserver->getErrorCount() << std::endl;
        
        // Reset to default
        logger.set_log_level(ulog::LogLevel::INFO);
        logger.remove_observer(cerrObserver);
        
        std::cout << "Log level filtering with cerr observer demo completed." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in filtering demo: " << e.what() << std::endl;
    }
}

/**
 * @brief Demo function showcasing exception safety with cerr observer
 */
void demo_cerr_observer_exception_safety() {
    std::cout << "\n=== DEMO: Cerr Observer Exception Safety ===" << std::endl;
    
    auto& logger = ulog::getLogger("ExceptionCerrDemo");
    
    auto cerrObserver = std::make_shared<CerrObserver>("ExceptionSafeHandler");
    
    std::cout << "Testing exception safety (stderr output should still occur)..." << std::endl;
    
    try {
        ulog::ObserverScope scope(logger, cerrObserver);
        
        logger.info("Starting risky operation - stdout");
        logger.warn("About to perform dangerous task - stdout");
        logger.error("Error detected in operation - stderr");
        
        // Simulate an exception
        throw std::runtime_error("Simulated critical error");
        
        logger.info("This won't be logged due to exception");
    }
    catch (const std::exception& e) {
        std::cerr << "[EXCEPTION] Caught exception: " << e.what() << std::endl;
        std::cout << "Exception caught. Observer should have processed error messages despite exception." << std::endl;
    }
    
    std::cout << "Exception safety demo completed. Error messages were sent to stderr." << std::endl;
}

/**
 * @brief Main demo function
 */
int main() {
    std::cout << "=== ulog Cerr Observer Demo ===" << std::endl;
    std::cout << "This demo showcases logging errors to stderr using the observer pattern." << std::endl;
    std::cout << "Normal messages (INFO, WARN, DEBUG) go to stdout," << std::endl;
    std::cout << "while ERROR and FATAL messages are redirected to stderr." << std::endl;
    
    // Run all cerr observer demos
    demo_basic_cerr_observer();
    demo_raii_cerr_observer();
    demo_multiple_observers_with_cerr();
    demo_cerr_observer_with_filtering();
    demo_cerr_observer_exception_safety();
    
    std::cout << "\n=== Demo Summary ===" << std::endl;
    std::cout << "The cerr observer demo has shown how to:" << std::endl;
    std::cout << "1. Create a custom observer that filters messages by level" << std::endl;
    std::cout << "2. Redirect ERROR and FATAL messages to stderr" << std::endl;
    std::cout << "3. Use RAII for automatic observer management" << std::endl;
    std::cout << "4. Combine multiple observers for different purposes" << std::endl;
    std::cout << "5. Handle log level filtering correctly" << std::endl;
    std::cout << "6. Ensure exception safety in observer usage" << std::endl;
    std::cout << "\nThis pattern is useful for separating error output from normal logging," << std::endl;
    std::cout << "allowing tools to capture and process errors separately." << std::endl;
    
    return 0;
}
