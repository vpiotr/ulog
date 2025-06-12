#include "utest/utest.h"
#include "ulog/ulog.h"
#include <sstream>
#include <thread>

namespace {
    // Test helper to capture console output
    class ConsoleCapture {
    public:
        ConsoleCapture() : old_cout(std::cout.rdbuf()) {
            std::cout.rdbuf(buffer.rdbuf());
        }
        
        ~ConsoleCapture() {
            std::cout.rdbuf(old_cout);
        }
        
        std::string get() {
            return buffer.str();
        }
        
        void clear() {
            buffer.str("");
            buffer.clear();
        }
        
    private:
        std::ostringstream buffer;
        std::streambuf* old_cout;
    };
}

UTEST_FUNC_DEF2(Logger, BasicLogging) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger("TestLogger");
    logger.info("Test message");
    
    std::string output = capture.get();
    UTEST_ASSERT_NOT_EQUALS(output.find("[INFO]"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("[TestLogger]"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("Test message"), std::string::npos);
}

UTEST_FUNC_DEF2(Logger, LogLevels) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger("LevelTest");
    
    // Set to TRACE to test all log levels
    logger.set_log_level(ulog::LogLevel::TRACE);
    
    logger.trace("Trace message");
    logger.debug("Debug message");
    logger.info("Info message");
    logger.warn("Warn message");
    logger.error("Error message");
    logger.fatal("Fatal message");
    
    std::string output = capture.get();
    UTEST_ASSERT_NOT_EQUALS(output.find("[TRACE]"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("[DEBUG]"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("[INFO]"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("[WARN]"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("[ERROR]"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("[FATAL]"), std::string::npos);
}

UTEST_FUNC_DEF2(Logger, GlobalLogger) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger();
    logger.info("Global message");
    
    std::string output = capture.get();
    UTEST_ASSERT_NOT_EQUALS(output.find("[INFO]"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("Global message"), std::string::npos);
    // Global logger should not have logger name in brackets
    UTEST_ASSERT_EQUALS(output.find("[]"), std::string::npos);
}

UTEST_FUNC_DEF2(Logger, ConsoleDisable) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger("DisableTest");
    logger.disable_console();
    logger.info("Should not appear");
    
    std::string output = capture.get();
    UTEST_ASSERT_EQUALS(output.find("Should not appear"), std::string::npos);
    
    logger.enable_console();
    logger.info("Should appear");
    
    output = capture.get();
    UTEST_ASSERT_NOT_EQUALS(output.find("Should appear"), std::string::npos);
}

UTEST_FUNC_DEF2(Logger, ParameterFormatting) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger("FormatTest");
    
    // Test anonymous parameters
    logger.info("User: {?}, Age: {?}", "John", 25);
    
    std::string output = capture.get();
    UTEST_ASSERT_NOT_EQUALS(output.find("User: John, Age: 25"), std::string::npos);
    
    capture.clear();
    
    // Test positional parameters
    logger.info("Name: {0}, Age: {1}, Name again: {0}", "Alice", 30);
    
    output = capture.get();
    UTEST_ASSERT_NOT_EQUALS(output.find("Name: Alice, Age: 30, Name again: Alice"), std::string::npos);
}

UTEST_FUNC_DEF2(Logger, ThreadSafety) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger("ThreadTest");
    
    std::vector<std::thread> threads;
    const int num_threads = 10;
    const int messages_per_thread = 10;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i, messages_per_thread]() {
            for (int j = 0; j < messages_per_thread; ++j) {
                logger.info("Thread {0} message {1}", i, j);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::string output = capture.get();
    
    // Count total messages
    size_t count = 0;
    size_t pos = 0;
    while ((pos = output.find("[INFO]", pos)) != std::string::npos) {
        ++count;
        ++pos;
    }
    
    UTEST_ASSERT_EQUALS(count, num_threads * messages_per_thread);
}

UTEST_FUNC_DEF2(Logger, RegistryConsistency) {
    auto& logger1 = ulog::getLogger("SameLogger");
    auto& logger2 = ulog::getLogger("SameLogger");
    
    // Should be the same instance
    UTEST_ASSERT_EQUALS(&logger1, &logger2);
    
    auto& global1 = ulog::getLogger();
    auto& global2 = ulog::getLogger();
    
    // Should be the same global instance
    UTEST_ASSERT_EQUALS(&global1, &global2);
}

UTEST_FUNC_DEF2(Logger, LogLevelFiltering) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger("FilterTest");
    
    // Default should be INFO
    UTEST_ASSERT_EQUALS(logger.get_log_level(), ulog::LogLevel::INFO);
    
    // Set to TRACE first to test all levels
    logger.set_log_level(ulog::LogLevel::TRACE);
    
    logger.trace("Should appear - trace");
    logger.debug("Should appear - debug");
    logger.info("Should appear - info");
    
    std::string output = capture.get();
    UTEST_ASSERT_NOT_EQUALS(output.find("Should appear - trace"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("Should appear - debug"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("Should appear - info"), std::string::npos);
    
    capture.clear();
    
    // Set level to INFO - should filter out TRACE and DEBUG
    logger.set_log_level(ulog::LogLevel::INFO);
    UTEST_ASSERT_EQUALS(logger.get_log_level(), ulog::LogLevel::INFO);
    
    logger.trace("Should NOT appear - trace");
    logger.debug("Should NOT appear - debug");
    logger.info("Should appear - info");
    logger.warn("Should appear - warn");
    logger.error("Should appear - error");
    
    output = capture.get();
    UTEST_ASSERT_EQUALS(output.find("Should NOT appear - trace"), std::string::npos);
    UTEST_ASSERT_EQUALS(output.find("Should NOT appear - debug"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("Should appear - info"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("Should appear - warn"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("Should appear - error"), std::string::npos);
    
    capture.clear();
    
    // Set level to ERROR - should filter out TRACE, DEBUG, INFO, WARN
    logger.set_log_level(ulog::LogLevel::ERROR);
    
    logger.trace("Should NOT appear - trace");
    logger.debug("Should NOT appear - debug");
    logger.info("Should NOT appear - info");
    logger.warn("Should NOT appear - warn");
    logger.error("Should appear - error");
    logger.fatal("Should appear - fatal");
    
    output = capture.get();
    UTEST_ASSERT_EQUALS(output.find("Should NOT appear"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("Should appear - error"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(output.find("Should appear - fatal"), std::string::npos);
    
    capture.clear();
    
    // Set level to OFF - should filter out all messages
    logger.set_log_level(ulog::LogLevel::OFF);
    
    logger.trace("Should NOT appear - trace");
    logger.debug("Should NOT appear - debug");
    logger.info("Should NOT appear - info");
    logger.warn("Should NOT appear - warn");
    logger.error("Should NOT appear - error");
    logger.fatal("Should NOT appear - fatal");
    
    output = capture.get();
    UTEST_ASSERT_EQUALS(output.find("Should NOT appear"), std::string::npos);
    UTEST_ASSERT_EQUALS(output.length(), 0);
}

UTEST_FUNC_DEF2(Logger, LogLevelFilteringWithBuffer) {
    auto& logger = ulog::getLogger("FilterBufferTest");
    
    // Enable buffer
    logger.enable_buffer(100);
    
    // Set level to WARN
    logger.set_log_level(ulog::LogLevel::WARN);
    
    logger.trace("Should NOT be buffered - trace");
    logger.debug("Should NOT be buffered - debug");
    logger.info("Should NOT be buffered - info");
    logger.warn("Should be buffered - warn");
    logger.error("Should be buffered - error");
    logger.fatal("Should be buffered - fatal");
    
    auto buffer = logger.buffer();
    UTEST_ASSERT_NOT_EQUALS(buffer, nullptr);
    
    // Should only have 3 messages (WARN, ERROR, FATAL)
    UTEST_ASSERT_EQUALS(buffer->size(), 3);
    
    // Check that the buffered messages are the correct ones
    bool found_warn = false, found_error = false, found_fatal = false;
    for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
        if (it->message.find("Should be buffered - warn") != std::string::npos) {
            found_warn = true;
            UTEST_ASSERT_EQUALS(it->level, ulog::LogLevel::WARN);
        }
        if (it->message.find("Should be buffered - error") != std::string::npos) {
            found_error = true;
            UTEST_ASSERT_EQUALS(it->level, ulog::LogLevel::ERROR);
        }
        if (it->message.find("Should be buffered - fatal") != std::string::npos) {
            found_fatal = true;
            UTEST_ASSERT_EQUALS(it->level, ulog::LogLevel::FATAL);
        }
    }
    
    UTEST_ASSERT_TRUE(found_warn);
    UTEST_ASSERT_TRUE(found_error);
    UTEST_ASSERT_TRUE(found_fatal);
    
    logger.disable_buffer();
}

UTEST_FUNC_DEF2(Logger, LogLevelFilteringWithObserver) {
    auto& logger = ulog::getLogger("FilterObserverTest");
    
    // Create a simple observer that counts messages
    class CountingObserver : public ulog::LogObserver {
    public:
        void handleNewMessage(const ulog::LogEntry& entry) override {
            count++;
            last_level = entry.level;
        }
        
        int count = 0;
        ulog::LogLevel last_level = ulog::LogLevel::TRACE;
    };
    
    auto observer = std::make_shared<CountingObserver>();
    logger.add_observer(observer);
    
    // Set level to INFO
    logger.set_log_level(ulog::LogLevel::INFO);
    
    logger.trace("Should NOT notify observer - trace");
    logger.debug("Should NOT notify observer - debug");
    logger.info("Should notify observer - info");
    logger.warn("Should notify observer - warn");
    
    // Should only have received 2 messages (INFO and WARN)
    UTEST_ASSERT_EQUALS(observer->count, 2);
    UTEST_ASSERT_EQUALS(observer->last_level, ulog::LogLevel::WARN);
    
    logger.remove_observer(observer);
}

void test_logger_register(bool& errorFound) {
    UTEST_FUNC2(Logger, BasicLogging);
    UTEST_FUNC2(Logger, LogLevels);
    UTEST_FUNC2(Logger, GlobalLogger);
    UTEST_FUNC2(Logger, ConsoleDisable);
    UTEST_FUNC2(Logger, ParameterFormatting);
    UTEST_FUNC2(Logger, ThreadSafety);
    UTEST_FUNC2(Logger, RegistryConsistency);
    UTEST_FUNC2(Logger, LogLevelFiltering);
    UTEST_FUNC2(Logger, LogLevelFilteringWithBuffer);
    UTEST_FUNC2(Logger, LogLevelFilteringWithObserver);
}

int main() {
    UTEST_PROLOG();
    
    test_logger_register(errorFound);
    
    UTEST_EPILOG();
    
    return 0;
}
