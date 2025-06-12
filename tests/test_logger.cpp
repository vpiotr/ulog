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

UTEST_FUNC_DEF2(Logger, AutoFlushingScope) {
    auto& logger = ulog::getLogger("AutoFlushTest");
    
    // Create a test observer that tracks flush calls
    class FlushTrackingObserver : public ulog::LogObserver {
    public:
        void handleNewMessage(const ulog::LogEntry& entry) override {
            (void)entry; // Unused
        }
        
        void handleFlush(const std::string& logger_name) override {
            flush_count++;
            last_flushed_logger = logger_name;
        }
        
        int flush_count = 0;
        std::string last_flushed_logger;
    };
    
    auto observer = std::make_shared<FlushTrackingObserver>();
    logger.add_observer(observer);
    
    // Initial state - no flushes
    UTEST_ASSERT_EQUALS(observer->flush_count, 0);
    
    {
        // Create AutoFlushingScope
        ulog::AutoFlushingScope scope(logger);
        
        // Log some messages
        logger.info("Message 1");
        logger.info("Message 2");
        
        // Still no flush yet
        UTEST_ASSERT_EQUALS(observer->flush_count, 0);
        
        // Scope will exit here and trigger flush
    }
    
    // After scope exit, flush should have been called once
    UTEST_ASSERT_EQUALS(observer->flush_count, 1);
    UTEST_ASSERT_EQUALS(observer->last_flushed_logger, "AutoFlushTest");
    
    // Test multiple nested scopes
    {
        ulog::AutoFlushingScope scope1(logger);
        {
            ulog::AutoFlushingScope scope2(logger);
            logger.info("Nested message");
        }
        // First nested scope exit should trigger one flush
        UTEST_ASSERT_EQUALS(observer->flush_count, 2);
    }
    // Second scope exit should trigger another flush
    UTEST_ASSERT_EQUALS(observer->flush_count, 3);
    
    logger.remove_observer(observer);
}

UTEST_FUNC_DEF2(Logger, CleanMessageBasic) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger("CleanMessageTest");
    logger.disable_console(); // We'll check the buffer instead
    logger.enable_buffer();
    
    // Test basic control character cleaning
    std::string message_with_newline = "Message with\nnewline";
    std::string message_with_tab = "Message with\ttab";
    std::string message_with_carriage_return = "Message with\rcarriage return";
    std::string message_with_null = std::string("Message with\0null", 17);
    
    logger.info(message_with_newline);
    logger.info(message_with_tab);
    logger.info(message_with_carriage_return);
    logger.info(message_with_null);
    
    auto buffer = logger.buffer();
    UTEST_ASSERT_NOT_NULL(buffer);
    UTEST_ASSERT_EQUALS(buffer->size(), 4);
    
    std::vector<std::string> messages;
    for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
        messages.push_back(it->message);
    }
    
    // Check that newline is replaced with space
    UTEST_ASSERT_NOT_EQUALS(messages[0].find("Message with newline"), std::string::npos);
    
    // Check that tab is replaced with space
    UTEST_ASSERT_NOT_EQUALS(messages[1].find("Message with tab"), std::string::npos);
    
    // Check that carriage return is replaced with space
    UTEST_ASSERT_NOT_EQUALS(messages[2].find("Message with carriage return"), std::string::npos);
    
    // Check that null is replaced with \x00 (not whitespace)
    UTEST_ASSERT_NOT_EQUALS(messages[3].find("Message with\\x00null"), std::string::npos);
    
    logger.disable_buffer();
}

UTEST_FUNC_DEF2(Logger, CleanMessageDisabled) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger("CleanMessageDisabledTest");
    logger.disable_console(); // We'll check the buffer instead
    logger.enable_buffer();
    
    // Disable message cleaning
    logger.disable_clean_message();
    UTEST_ASSERT_FALSE(logger.is_clean_message_enabled());
    
    // Test that control characters are NOT cleaned when disabled
    std::string message_with_newline = "Message with\nnewline";
    logger.info(message_with_newline);
    
    auto buffer = logger.buffer();
    UTEST_ASSERT_NOT_NULL(buffer);
    UTEST_ASSERT_EQUALS(buffer->size(), 1);
    
    std::vector<std::string> messages;
    for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
        messages.push_back(it->message);
    }
    
    // Check that newline is NOT replaced when cleaning is disabled
    UTEST_ASSERT_NOT_EQUALS(messages[0].find("Message with\nnewline"), std::string::npos);
    UTEST_ASSERT_EQUALS(messages[0].find("\\x0A"), std::string::npos);
    
    // Re-enable message cleaning for other tests
    logger.enable_clean_message();
    UTEST_ASSERT_TRUE(logger.is_clean_message_enabled());
    
    logger.disable_buffer();
}

UTEST_FUNC_DEF2(Logger, CleanMessageUnicode) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger("CleanMessageUnicodeTest");
    logger.disable_console(); // We'll check the buffer instead
    logger.enable_buffer();
    
    // Test that Unicode characters are preserved
    std::string unicode_message = "Message with unicode: ñáéíóú 中文 🙂 Ω α β γ";
    std::string mixed_message = "Unicode: ñáéíóú\nwith newline\tand tab";
    
    logger.info(unicode_message);
    logger.info(mixed_message);
    
    auto buffer = logger.buffer();
    UTEST_ASSERT_NOT_NULL(buffer);
    UTEST_ASSERT_EQUALS(buffer->size(), 2);
    
    std::vector<std::string> messages;
    for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
        messages.push_back(it->message);
    }
    
    // Check that Unicode characters are preserved
    UTEST_ASSERT_NOT_EQUALS(messages[0].find("ñáéíóú"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(messages[0].find("中文"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(messages[0].find("🙂"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(messages[0].find("Ω α β γ"), std::string::npos);
    
    // Check mixed message: Unicode preserved, control chars cleaned
    UTEST_ASSERT_NOT_EQUALS(messages[1].find("ñáéíóú"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(messages[1].find("with newline and tab"), std::string::npos); // whitespace converted to spaces
    
    logger.disable_buffer();
}

UTEST_FUNC_DEF2(Logger, CleanMessageAllControlChars) {
    ConsoleCapture capture;
    
    auto& logger = ulog::getLogger("CleanMessageAllControlTest");
    logger.disable_console(); // We'll check the buffer instead
    logger.enable_buffer();
    
    // Test all control characters from 0 to 31
    std::string message = "Control chars: ";
    for (int i = 0; i < 32; ++i) {
        message += static_cast<char>(i);
    }
    message += " End";
    
    logger.info(message);
    
    auto buffer = logger.buffer();
    UTEST_ASSERT_NOT_NULL(buffer);
    UTEST_ASSERT_EQUALS(buffer->size(), 1);
    
    std::vector<std::string> messages;
    for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
        messages.push_back(it->message);
    }
    std::string cleaned_message = messages[0];
    
    // Check that the message contains hex codes for non-whitespace control characters
    UTEST_ASSERT_NOT_EQUALS(cleaned_message.find("Control chars: "), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(cleaned_message.find("\\x00"), std::string::npos); // NULL
    UTEST_ASSERT_NOT_EQUALS(cleaned_message.find("\\x01"), std::string::npos); // SOH
    UTEST_ASSERT_NOT_EQUALS(cleaned_message.find("\\x1F"), std::string::npos); // US (Unit Separator)
    UTEST_ASSERT_NOT_EQUALS(cleaned_message.find(" End"), std::string::npos);
    
    // Check that whitespace characters are replaced with spaces (not hex codes)
    UTEST_ASSERT_EQUALS(cleaned_message.find("\\x09"), std::string::npos); // tab should not be hex-encoded
    UTEST_ASSERT_EQUALS(cleaned_message.find("\\x0A"), std::string::npos); // newline should not be hex-encoded
    UTEST_ASSERT_EQUALS(cleaned_message.find("\\x0D"), std::string::npos); // carriage return should not be hex-encoded
    
    // Check that regular ASCII characters (32 and above) are not affected
    UTEST_ASSERT_EQUALS(cleaned_message.find("\\x20"), std::string::npos); // Space should not be encoded
    
    logger.disable_buffer();
}

UTEST_FUNC_DEF2(Logger, CleanMessageWithObserver) {
    // Test that cleaned messages are also passed to observers
    class TestObserver : public ulog::LogObserver {
    public:
        void handleNewMessage(const ulog::LogEntry& entry) override {
            last_message = entry.message;
        }
        std::string last_message;
    };
    
    auto& logger = ulog::getLogger("CleanMessageObserverTest");
    logger.disable_console();
    
    auto observer = std::make_shared<TestObserver>();
    logger.add_observer(observer);
    
    // Create message with explicit control characters
    std::string message_with_control_chars = "Message\nwith\ttabs\rand";
    message_with_control_chars += static_cast<char>(1);  // SOH (\x01)
    message_with_control_chars += "control";
    message_with_control_chars += static_cast<char>(2);  // STX (\x02) 
    message_with_control_chars += "chars";
    
    logger.info(message_with_control_chars);
    
    // Check that observer received cleaned message
    // Whitespace characters should be replaced with spaces
    UTEST_ASSERT_NOT_EQUALS(observer->last_message.find("Message with tabs and"), std::string::npos);
    // Non-whitespace control characters should be hex-encoded
    UTEST_ASSERT_NOT_EQUALS(observer->last_message.find("\\x01"), std::string::npos); // SOH
    UTEST_ASSERT_NOT_EQUALS(observer->last_message.find("\\x02"), std::string::npos); // STX
    
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
    UTEST_FUNC2(Logger, AutoFlushingScope);
    UTEST_FUNC2(Logger, CleanMessageBasic);
    UTEST_FUNC2(Logger, CleanMessageDisabled);
    UTEST_FUNC2(Logger, CleanMessageUnicode);
    UTEST_FUNC2(Logger, CleanMessageAllControlChars);
    UTEST_FUNC2(Logger, CleanMessageWithObserver);
}

int main() {
    UTEST_PROLOG();
    
    test_logger_register(errorFound);
    
    UTEST_EPILOG();
    
    return 0;
}
