#include "ulog/ulog.h"
#include <thread>
#include <chrono>
#include <random>

/**
 * @brief Custom observer that counts messages by level
 */
class MessageCounterObserver : public ulog::LogObserver {
public:
    void handleNewMessage(const ulog::LogEntry& entry) override {
        std::lock_guard<std::mutex> lock(mutex_);
        counts_[entry.level]++;
        std::cout << "[OBSERVER] Received " << ulog::to_string(entry.level) 
                  << " message from [" << entry.logger_name << "]" << std::endl;
    }
    
    void handleRegistered(const std::string& logger_name) override {
        std::cout << "[OBSERVER] Registered to logger: " << logger_name << std::endl;
    }
    
    void handleUnregistered(const std::string& logger_name) override {
        std::cout << "[OBSERVER] Unregistered from logger: " << logger_name << std::endl;
    }
    
    void printStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "\n=== Message Statistics ===" << std::endl;
        for (const auto& [level, count] : counts_) {
            std::cout << ulog::to_string(level) << ": " << count << " messages" << std::endl;
        }
        std::cout << "=========================" << std::endl;
    }
    
private:
    mutable std::mutex mutex_;
    std::unordered_map<ulog::LogLevel, int> counts_;
};

/**
 * @brief Demo function showcasing basic logging features
 */
void demo_basic_logging() {
    std::cout << "\n=== DEMO: Basic Logging ===" << std::endl;
    
    // Get global logger
    auto& globalLogger = ulog::getLogger();
    globalLogger.info("This is a global logger message");
    
    // Get named logger
    auto& appLogger = ulog::getLogger("DemoApp");
    
    // Test all log levels
    appLogger.trace("Detailed trace information");
    appLogger.debug("Debug information for troubleshooting");
    appLogger.info("General information about application flow");
    appLogger.warn("Warning: something might be wrong");
    appLogger.error("Error: something went wrong");
    appLogger.fatal("Fatal: application cannot continue");
    
    // Test parameter formatting
    std::string username = "Alice";
    int age = 30;
    double score = 95.7;
    
    appLogger.info("User profile - Name: {?}, Age: {?}, Score: {?}", username, age, score);
    appLogger.info("Positional format - {0} scored {2} points at age {1}", username, age, score);
    
    appLogger.flush();
}

/**
 * @brief Demo function showcasing memory buffer functionality
 */
void demo_buffer_functionality() {
    std::cout << "\n=== DEMO: Buffer Functionality ===" << std::endl;
    
    auto& logger = ulog::getLogger("BufferDemo");
    logger.disable_console(); // Disable console to focus on buffer
    
    // Enable buffer with capacity of 5 messages
    logger.enable_buffer(5);
    
    // Log some messages
    for (int i = 1; i <= 8; ++i) {
        logger.info("Buffer message {0}", i);
    }
    
    // Print buffer contents (should only contain last 5 messages due to capacity)
    auto buffer = logger.buffer();
    std::cout << "Buffer contains " << buffer->size() << " messages:" << std::endl;
    
    for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
        std::cout << "  " << it->formatted_message() << std::endl;
    }
    
    logger.clear_buffer();
    std::cout << "After clearing: " << buffer->size() << " messages" << std::endl;
    
    logger.enable_console();
    logger.disable_buffer();
    logger.info("Buffer demo completed");
}

/**
 * @brief Demo function showcasing observer pattern
 */
void demo_observer_pattern() {
    std::cout << "\n=== DEMO: Observer Pattern ===" << std::endl;
    
    auto& logger = ulog::getLogger("ObserverDemo");
    auto observer = std::make_shared<MessageCounterObserver>();
    
    // Manual observer management
    logger.add_observer(observer);
    
    logger.info("First observed message");
    logger.warn("Warning message");
    logger.error("Error message");
    
    logger.remove_observer(observer);
    logger.info("This message won't be observed");
    
    // RAII observer management
    {
        ulog::observer_scope scope(logger, observer);
        logger.info("RAII scoped message 1");
        logger.debug("RAII scoped message 2");
    } // Observer automatically removed here
    
    logger.info("Message after scope ends - not observed");
    
    observer->printStats();
}

/**
 * @brief Demo function showcasing console control
 */
void demo_console_control() {
    std::cout << "\n=== DEMO: Console Control ===" << std::endl;
    
    auto& logger = ulog::getLogger("ConsoleDemo");
    
    logger.info("Console is enabled - you can see this");
    
    logger.disable_console();
    logger.info("Console is disabled - you cannot see this in console");
    
    // Enable buffer to capture the message that didn't go to console
    logger.enable_buffer(10);
    logger.warn("This warning is buffered but not on console");
    
    logger.enable_console();
    logger.info("Console is re-enabled - you can see this again");
    
    // Show what was captured in buffer
    auto buffer = logger.buffer();
    std::cout << "Messages captured while console was disabled:" << std::endl;
    for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
        if (it->message.find("disabled") != std::string::npos || 
            it->message.find("buffered") != std::string::npos) {
            std::cout << "  " << it->formatted_message() << std::endl;
        }
    }
    
    logger.disable_buffer();
}

/**
 * @brief Demo function showcasing thread safety
 */
void demo_thread_safety() {
    std::cout << "\n=== DEMO: Thread Safety ===" << std::endl;
    
    auto& logger = ulog::getLogger("ThreadDemo");
    logger.enable_buffer(0); // Unlimited buffer
    
    const int num_threads = 5;
    const int messages_per_thread = 20;
    
    std::vector<std::thread> threads;
    
    // Create multiple threads that log concurrently
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i, messages_per_thread]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(10, 100);
            
            for (int j = 0; j < messages_per_thread; ++j) {
                // Random delay to increase chance of race conditions
                std::this_thread::sleep_for(std::chrono::microseconds(dis(gen)));
                
                switch (j % 4) {
                    case 0: logger.info("Thread {0} info message {1}", i, j); break;
                    case 1: logger.debug("Thread {0} debug message {1}", i, j); break;
                    case 2: logger.warn("Thread {0} warning message {1}", i, j); break;
                    case 3: logger.error("Thread {0} error message {1}", i, j); break;
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto buffer = logger.buffer();
    std::cout << "Total messages logged by " << num_threads << " threads: " 
              << buffer->size() << std::endl;
    std::cout << "Expected: " << (num_threads * messages_per_thread) << std::endl;
    
    // Verify all messages are properly formatted
    int malformed_count = 0;
    for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
        std::string formatted = it->formatted_message();
        if (formatted.find("[") == std::string::npos || 
            formatted.find("]") == std::string::npos) {
            malformed_count++;
        }
    }
    
    std::cout << "Malformed messages: " << malformed_count << std::endl;
    logger.disable_buffer();
}

/**
 * @brief Demo function showcasing logger factory usage
 */
void demo_logger_factory() {
    std::cout << "\n=== DEMO: Logger Factory ===" << std::endl;
    
    // Factory that creates logger with specific configuration
    auto factory = [](const std::string& name) {
        auto logger = std::make_unique<ulog::Logger>(name);
        logger->enable_buffer(50); // All factory loggers have buffer enabled
        return logger;
    };
    
    // Create logger using factory via registry
    auto& factoryLogger = ulog::LoggerRegistry::instance().get_logger("FactoryLogger", factory);
    factoryLogger.info("Logger created with factory - buffer should be enabled");
    
    // Verify buffer is enabled
    if (factoryLogger.buffer()) {
        std::cout << "Factory logger has buffer enabled with " 
                  << factoryLogger.buffer()->size() << " messages" << std::endl;
    }
    
    // For global logger, just use regular interface since factory pattern is more complex
    auto& globalLogger = ulog::getLogger();
    globalLogger.info("Message from global logger");
}

/**
 * @brief Main demo function
 */
int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "           ulog Library Demonstration" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    try {
        demo_basic_logging();
        demo_buffer_functionality();
        demo_observer_pattern();
        demo_console_control();
        demo_thread_safety();
        demo_logger_factory();
        
        std::cout << "\n=== DEMO COMPLETED SUCCESSFULLY ===" << std::endl;
        std::cout << "All core functionalities demonstrated!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
