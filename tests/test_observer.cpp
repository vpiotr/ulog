#include "utest/utest.h"
#include "ulog/ulog.h"
#include <vector>

namespace {
    class TestObserver : public ulog::LogObserver {
    public:
        void handleRegistered(const std::string& logger_name) override {
            registered_loggers.push_back(logger_name);
        }
        
        void handleUnregistered(const std::string& logger_name) override {
            unregistered_loggers.push_back(logger_name);
        }
        
        void handleNewMessage(const ulog::LogEntry& entry) override {
            messages.push_back(entry);
        }
        
        void handleFlush(const std::string& logger_name) override {
            flushed_loggers.push_back(logger_name);
        }
        
        std::vector<std::string> registered_loggers;
        std::vector<std::string> unregistered_loggers;
        std::vector<ulog::LogEntry> messages;
        std::vector<std::string> flushed_loggers;
    };
}

UTEST_FUNC_DEF2(Observer, Registration) {
    auto& logger = ulog::getLogger("ObserverTest");
    logger.disable_console();
    
    auto observer = std::make_shared<TestObserver>();
    
    logger.add_observer(observer);
    
    UTEST_ASSERT_EQUALS(observer->registered_loggers.size(), 1);
    UTEST_ASSERT_EQUALS(observer->registered_loggers[0], "ObserverTest");
    
    logger.remove_observer(observer);
    
    UTEST_ASSERT_EQUALS(observer->unregistered_loggers.size(), 1);
    UTEST_ASSERT_EQUALS(observer->unregistered_loggers[0], "ObserverTest");
}

UTEST_FUNC_DEF2(Observer, MessageNotification) {
    auto& logger = ulog::getLogger("MessageTest");
    logger.disable_console();
    
    auto observer = std::make_shared<TestObserver>();
    logger.add_observer(observer);
    
    logger.info("Test message 1");
    logger.debug("Test message 2");
    logger.error("Test message 3");
    
    UTEST_ASSERT_EQUALS(observer->messages.size(), 3);
    UTEST_ASSERT_EQUALS(observer->messages[0].message, "Test message 1");
    UTEST_ASSERT_EQUALS(observer->messages[1].message, "Test message 2");
    UTEST_ASSERT_EQUALS(observer->messages[2].message, "Test message 3");
    
    UTEST_ASSERT_EQUALS(observer->messages[0].level, ulog::LogLevel::INFO);
    UTEST_ASSERT_EQUALS(observer->messages[1].level, ulog::LogLevel::DEBUG);
    UTEST_ASSERT_EQUALS(observer->messages[2].level, ulog::LogLevel::ERROR);
    
    logger.remove_observer(observer);
}

UTEST_FUNC_DEF2(Observer, FlushNotification) {
    auto& logger = ulog::getLogger("FlushTest");
    logger.disable_console();
    
    auto observer = std::make_shared<TestObserver>();
    logger.add_observer(observer);
    
    logger.flush();
    
    UTEST_ASSERT_EQUALS(observer->flushed_loggers.size(), 1);
    UTEST_ASSERT_EQUALS(observer->flushed_loggers[0], "FlushTest");
    
    logger.remove_observer(observer);
}

UTEST_FUNC_DEF2(Observer, MultipleObservers) {
    auto& logger = ulog::getLogger("MultiTest");
    logger.disable_console();
    
    auto observer1 = std::make_shared<TestObserver>();
    auto observer2 = std::make_shared<TestObserver>();
    
    logger.add_observer(observer1);
    logger.add_observer(observer2);
    
    logger.info("Broadcast message");
    
    UTEST_ASSERT_EQUALS(observer1->messages.size(), 1);
    UTEST_ASSERT_EQUALS(observer2->messages.size(), 1);
    UTEST_ASSERT_EQUALS(observer1->messages[0].message, "Broadcast message");
    UTEST_ASSERT_EQUALS(observer2->messages[0].message, "Broadcast message");
    
    logger.remove_observer(observer1);
    logger.remove_observer(observer2);
}

UTEST_FUNC_DEF2(Observer, ScopeRAII) {
    auto& logger = ulog::getLogger("ScopeTest");
    logger.disable_console();
    
    auto observer = std::make_shared<TestObserver>();
    
    {
        ulog::ObserverScope scope(logger, observer);
        
        // Observer should be registered
        UTEST_ASSERT_EQUALS(observer->registered_loggers.size(), 1);
        
        logger.info("Scoped message");
        UTEST_ASSERT_EQUALS(observer->messages.size(), 1);
    }
    
    // Observer should be automatically unregistered
    UTEST_ASSERT_EQUALS(observer->unregistered_loggers.size(), 1);
    
    // New message should not reach observer
    logger.info("Post-scope message");
    UTEST_ASSERT_EQUALS(observer->messages.size(), 1); // Still only one message
}

UTEST_FUNC_DEF2(Observer, LogEntryContent) {
    auto& logger = ulog::getLogger("ContentTest");
    logger.disable_console();
    
    auto observer = std::make_shared<TestObserver>();
    logger.add_observer(observer);
    
    logger.warn("Warning message");
    
    UTEST_ASSERT_EQUALS(observer->messages.size(), 1);
    
    const auto& entry = observer->messages[0];
    UTEST_ASSERT_EQUALS(entry.level, ulog::LogLevel::WARN);
    UTEST_ASSERT_EQUALS(entry.logger_name, "ContentTest");
    UTEST_ASSERT_EQUALS(entry.message, "Warning message");
    
    // Check that formatted message contains all components
    std::string formatted = entry.formatted_message();
    UTEST_ASSERT_NOT_EQUALS(formatted.find("[WARN]"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(formatted.find("[ContentTest]"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(formatted.find("Warning message"), std::string::npos);
    
    logger.remove_observer(observer);
}

void test_observer_register(bool& errorFound) {
    UTEST_FUNC2(Observer, Registration);
    UTEST_FUNC2(Observer, MessageNotification);
    UTEST_FUNC2(Observer, FlushNotification);
    UTEST_FUNC2(Observer, MultipleObservers);
    UTEST_FUNC2(Observer, ScopeRAII);
    UTEST_FUNC2(Observer, LogEntryContent);
}

int main() {
    UTEST_PROLOG();
    
    test_observer_register(errorFound);
    
    UTEST_EPILOG();
    
    return 0;
}
