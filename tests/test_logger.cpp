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

void test_logger_register(bool& errorFound) {
    UTEST_FUNC2(Logger, BasicLogging);
    UTEST_FUNC2(Logger, LogLevels);
    UTEST_FUNC2(Logger, GlobalLogger);
    UTEST_FUNC2(Logger, ConsoleDisable);
    UTEST_FUNC2(Logger, ParameterFormatting);
    UTEST_FUNC2(Logger, ThreadSafety);
    UTEST_FUNC2(Logger, RegistryConsistency);
}

int main() {
    UTEST_PROLOG();
    
    test_logger_register(errorFound);
    
    UTEST_EPILOG();
    
    return 0;
}
