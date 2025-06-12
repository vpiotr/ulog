/**
 * @file demo_auto_flushing.cpp
 * @brief Demonstration of AutoFlushingScope RAII functionality
 * 
 * This demo showcases the AutoFlushingScope class which provides automatic
 * flushing of loggers when the scope exits.
 */

#include "ulog/ulog.h"
#include <iostream>
#include <thread>
#include <chrono>

/**
 * @brief File observer that tracks flush operations
 */
class FlushObserver : public ulog::LogObserver {
public:
    explicit FlushObserver(const std::string& name) : name_(name) {}
    
    void handleNewMessage(const ulog::LogEntry& entry) override {
        // We don't need to handle messages in this demo
        (void)entry;
    }
    
    void handleFlush(const std::string& logger_name) override {
        std::cout << "[" << name_ << "] Logger '" << logger_name << "' was flushed" << std::endl;
        flush_count_++;
    }
    
    int getFlushCount() const { return flush_count_; }
    
private:
    std::string name_;
    int flush_count_ = 0;
};

/**
 * @brief Demo function showing basic AutoFlushingScope usage
 */
void demo_basic_auto_flushing() {
    std::cout << "\n=== DEMO: Basic AutoFlushingScope Usage ===" << std::endl;
    
    auto& logger = ulog::getLogger("BasicAutoFlush");
    
    // Add observer to track flush operations
    auto observer = std::make_shared<FlushObserver>("FlushTracker");
    logger.add_observer(observer);
    
    std::cout << "Initial flush count: " << observer->getFlushCount() << std::endl;
    
    std::cout << "\nEntering auto-flushing scope..." << std::endl;
    {
        ulog::AutoFlushingScope scope(logger);
        
        logger.info("Message 1 - inside auto-flushing scope");
        logger.warn("Message 2 - still inside scope");
        logger.error("Message 3 - about to exit scope");
        
        std::cout << "About to exit scope (flush will happen automatically)..." << std::endl;
    }
    std::cout << "Exited scope - flush should have occurred" << std::endl;
    std::cout << "Final flush count: " << observer->getFlushCount() << std::endl;
    
    logger.remove_observer(observer);
}

/**
 * @brief Demo function showing nested AutoFlushingScope usage
 */
void demo_nested_auto_flushing() {
    std::cout << "\n=== DEMO: Nested AutoFlushingScope Usage ===" << std::endl;
    
    auto& logger = ulog::getLogger("NestedAutoFlush");
    
    // Add observer to track flush operations
    auto observer = std::make_shared<FlushObserver>("NestedTracker");
    logger.add_observer(observer);
    
    std::cout << "Starting nested scopes demo..." << std::endl;
    std::cout << "Initial flush count: " << observer->getFlushCount() << std::endl;
    
    {
        ulog::AutoFlushingScope outerScope(logger);
        logger.info("Outer scope - message 1");
        
        {
            ulog::AutoFlushingScope innerScope(logger);
            logger.info("Inner scope - message 1");
            logger.info("Inner scope - message 2");
            std::cout << "Exiting inner scope..." << std::endl;
        }
        std::cout << "Inner scope exited, flush count: " << observer->getFlushCount() << std::endl;
        
        logger.info("Outer scope - message 2 (after inner scope)");
        std::cout << "Exiting outer scope..." << std::endl;
    }
    std::cout << "Outer scope exited, final flush count: " << observer->getFlushCount() << std::endl;
    
    logger.remove_observer(observer);
}

/**
 * @brief Demo function showing AutoFlushingScope with different loggers
 */
void demo_multiple_loggers() {
    std::cout << "\n=== DEMO: AutoFlushingScope with Multiple Loggers ===" << std::endl;
    
    auto& logger1 = ulog::getLogger("Logger1");
    auto& logger2 = ulog::getLogger("Logger2");
    
    // Add observers to both loggers
    auto observer1 = std::make_shared<FlushObserver>("Tracker1");
    auto observer2 = std::make_shared<FlushObserver>("Tracker2");
    
    logger1.add_observer(observer1);
    logger2.add_observer(observer2);
    
    std::cout << "Using AutoFlushingScope with different loggers..." << std::endl;
    
    {
        ulog::AutoFlushingScope scope1(logger1);
        ulog::AutoFlushingScope scope2(logger2);
        
        logger1.info("Message from Logger1");
        logger2.warn("Message from Logger2");
        
        std::cout << "Both scopes will exit and flush their respective loggers..." << std::endl;
    }
    
    std::cout << "Logger1 flush count: " << observer1->getFlushCount() << std::endl;
    std::cout << "Logger2 flush count: " << observer2->getFlushCount() << std::endl;
    
    logger1.remove_observer(observer1);
    logger2.remove_observer(observer2);
}

/**
 * @brief Demo function showing AutoFlushingScope with error handling
 */
void demo_exception_safety() {
    std::cout << "\n=== DEMO: AutoFlushingScope Exception Safety ===" << std::endl;
    
    auto& logger = ulog::getLogger("ExceptionTest");
    
    auto observer = std::make_shared<FlushObserver>("ExceptionTracker");
    logger.add_observer(observer);
    
    std::cout << "Testing exception safety (flush should still occur)..." << std::endl;
    
    try {
        ulog::AutoFlushingScope scope(logger);
        
        logger.info("Message before exception");
        logger.warn("About to throw exception...");
        
        // Simulate an exception
        throw std::runtime_error("Simulated error");
        
        logger.error("This message won't be logged due to exception");
    }
    catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
        std::cout << "AutoFlushingScope should have flushed despite exception" << std::endl;
    }
    
    std::cout << "Exception handling flush count: " << observer->getFlushCount() << std::endl;
    
    logger.remove_observer(observer);
}

/**
 * @brief Main demo function that runs all AutoFlushingScope demonstrations
 */
void demo_auto_flushing_main() {
    std::cout << "=== ulog AutoFlushingScope Demonstrations ===" << std::endl;
    std::cout << "This demo showcases the RAII AutoFlushingScope functionality" << std::endl;
    std::cout << "which automatically flushes loggers when scopes exit." << std::endl;
    
    demo_basic_auto_flushing();
    demo_nested_auto_flushing();
    demo_multiple_loggers();
    demo_exception_safety();
    
    std::cout << "\n=== All AutoFlushingScope Demos Completed ===" << std::endl;
}

int main() {
    demo_auto_flushing_main();
    return 0;
}
