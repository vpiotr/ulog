/**
 * @file demo_debug_scope.cpp
 * @brief Demonstration of DebugScope RAII functionality with observer pattern
 * 
 * This demo showcases the DebugScope extension that automatically logs "Entering: x" 
 * and "Exiting: x" messages for labeled scopes using RAII pattern combined
 * with observer management.
 */

#include "ulog/ulog.h"
#include "extensions/include/debug_scope.h"
#include <iostream>
#include <memory>
#include <string>

using ulog::extensions::DebugObserver;
using ulog::extensions::DebugScope;

/**
 * @brief Demo function showing basic DebugScope usage
 */
void demo_basic_debug_scope() {
    std::cout << "\n=== DEMO: Basic DebugScope Usage ===" << std::endl;
    
    auto& logger = ulog::getLogger("DebugScopeDemo");
    logger.set_log_level(ulog::LogLevel::DEBUG); // Ensure debug messages are shown
    
    auto observer = std::make_shared<DebugObserver>("BasicTracker");
    
    {
        ulog::ObserverScope observerScope(logger, observer);
        
        std::cout << "Creating debug scope 'main_function'..." << std::endl;
        {
            DebugScope scope(logger, "main_function");
            
            logger.info("Doing some work in main function");
            logger.debug("Debug information in main function");
            
            std::cout << "About to exit main_function scope..." << std::endl;
        }
        std::cout << "Exited main_function scope" << std::endl;
        
    } // Observer automatically removed here
    
    observer->printCapturedMessages();
}

/**
 * @brief Demo function showing nested DebugScope usage
 */
void demo_nested_debug_scope() {
    std::cout << "\n=== DEMO: Nested DebugScope Usage ===" << std::endl;
    
    auto& logger = ulog::getLogger("NestedDebugDemo");
    logger.set_log_level(ulog::LogLevel::DEBUG);
    
    auto observer = std::make_shared<DebugObserver>("NestedTracker");
    
    {
        ulog::ObserverScope observerScope(logger, observer);
        
        std::cout << "Starting nested scopes demo..." << std::endl;
        {
            DebugScope outerScope(logger, "outer_function");
            
            logger.info("In outer function");
            
            {
                DebugScope innerScope(logger, "inner_function");
                
                logger.info("In inner function");
                logger.debug("Processing data in inner function");
                
                {
                    DebugScope deepScope(logger, "deep_function");
                    logger.debug("Deep processing");
                }
                
                logger.info("Continuing in inner function");
            }
            
            logger.info("Back in outer function");
        }
        std::cout << "All nested scopes completed" << std::endl;
        
    } // Observer automatically removed here
    
    observer->printCapturedMessages();
}

/**
 * @brief Demo function showing DebugScope with multiple loggers
 */
void demo_multiple_logger_debug_scope() {
    std::cout << "\n=== DEMO: DebugScope with Multiple Loggers ===" << std::endl;
    
    auto& logger1 = ulog::getLogger("Service1");
    auto& logger2 = ulog::getLogger("Service2");
    
    logger1.set_log_level(ulog::LogLevel::DEBUG);
    logger2.set_log_level(ulog::LogLevel::DEBUG);
    
    auto observer1 = std::make_shared<DebugObserver>("Service1Tracker");
    auto observer2 = std::make_shared<DebugObserver>("Service2Tracker");
    
    {
        ulog::ObserverScope scope1(logger1, observer1);
        ulog::ObserverScope scope2(logger2, observer2);
        
        std::cout << "Using DebugScope with different loggers..." << std::endl;
        
        {
            DebugScope service1Scope(logger1, "initialize_service1");
            DebugScope service2Scope(logger2, "initialize_service2");
            
            logger1.info("Service1 initializing...");
            logger2.info("Service2 initializing...");
            
            {
                DebugScope service1Work(logger1, "process_data");
                logger1.debug("Processing data in service1");
            }
            
            {
                DebugScope service2Work(logger2, "handle_requests");
                logger2.debug("Handling requests in service2");
            }
            
            logger1.info("Service1 ready");
            logger2.info("Service2 ready");
        }
        
    } // Observers automatically removed here
    
    std::cout << "\nService1 scope messages:" << std::endl;
    observer1->printCapturedMessages();
    
    std::cout << "\nService2 scope messages:" << std::endl;
    observer2->printCapturedMessages();
}

/**
 * @brief Demo function showing DebugScope exception safety
 */
void demo_debug_scope_exception_safety() {
    std::cout << "\n=== DEMO: DebugScope Exception Safety ===" << std::endl;
    
    auto& logger = ulog::getLogger("ExceptionDemo");
    logger.set_log_level(ulog::LogLevel::DEBUG);
    
    auto observer = std::make_shared<DebugObserver>("ExceptionTracker");
    
    {
        ulog::ObserverScope observerScope(logger, observer);
        
        std::cout << "Testing exception safety (exit message should still be logged)..." << std::endl;
        
        try {
            DebugScope scope(logger, "risky_operation");
            
            logger.info("Starting risky operation");
            logger.debug("About to throw exception...");
            
            // Simulate an exception
            throw std::runtime_error("Simulated error in risky operation");
            
            logger.info("This won't be logged due to exception");
        }
        catch (const std::exception& e) {
            std::cout << "Caught exception: " << e.what() << std::endl;
            std::cout << "DebugScope should have logged exit despite exception" << std::endl;
        }
        
    } // Observer automatically removed here
    
    observer->printCapturedMessages();
}

/**
 * @brief Demo function showing DebugScope with conditional debugging
 */
void demo_conditional_debug_scope() {
    std::cout << "\n=== DEMO: Conditional DebugScope (with different log levels) ===" << std::endl;
    
    auto& logger = ulog::getLogger("ConditionalDemo");
    
    auto observer = std::make_shared<DebugObserver>("ConditionalTracker");
    
    {
        ulog::ObserverScope observerScope(logger, observer);
        
        // Test with INFO level (should not see debug scope messages)
        std::cout << "\nTesting with INFO log level (debug scope messages filtered):" << std::endl;
        logger.set_log_level(ulog::LogLevel::INFO);
        
        {
            DebugScope scope(logger, "filtered_scope");
            logger.info("This info message will appear");
            logger.debug("This debug message will be filtered");
        }
        
        std::cout << "Messages captured with INFO level: " << observer->getMessageCount() << std::endl;
        
        // Test with DEBUG level (should see debug scope messages)
        std::cout << "\nTesting with DEBUG log level (debug scope messages visible):" << std::endl;
        logger.set_log_level(ulog::LogLevel::DEBUG);
        
        {
            DebugScope scope(logger, "visible_scope");
            logger.info("This info message will appear");
            logger.debug("This debug message will appear");
        }
        
    } // Observer automatically removed here
    
    observer->printCapturedMessages();
    std::cout << "Total messages captured: " << observer->getMessageCount() << std::endl;
}

/**
 * @brief Main demo function that runs all DebugScope demonstrations
 */
void demo_debug_scope_main() {
    std::cout << "=== ulog DebugScope + Observer RAII Demonstrations ===" << std::endl;
    std::cout << "This demo showcases the DebugScope RAII functionality combined" << std::endl;
    std::cout << "with observer pattern for automatic scope entry/exit logging." << std::endl;
    
    demo_basic_debug_scope();
    demo_nested_debug_scope();
    demo_multiple_logger_debug_scope();
    demo_debug_scope_exception_safety();
    demo_conditional_debug_scope();
    
    std::cout << "\n=== All DebugScope + Observer Demos Completed ===" << std::endl;
}

int main() {
    demo_debug_scope_main();
    return 0;
}
