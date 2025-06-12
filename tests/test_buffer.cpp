#include "utest/utest.h"
#include "ulog/ulog.h"
#include <thread>

UTEST_FUNC_DEF2(Buffer, EnableDisable) {
    auto& logger = ulog::getLogger("BufferTest");
    
    // Initially no buffer
    UTEST_ASSERT_EQUALS(logger.buffer(), nullptr);
    
    logger.enable_buffer(100);
    UTEST_ASSERT_NOT_EQUALS(logger.buffer(), nullptr);
    
    logger.disable_buffer();
    UTEST_ASSERT_EQUALS(logger.buffer(), nullptr);
}

UTEST_FUNC_DEF2(Buffer, Storage) {
    auto& logger = ulog::getLogger("StorageTest");
    logger.disable_console(); // Disable console to avoid output during test
    
    logger.enable_buffer(0); // Unlimited capacity
    
    logger.info("Message 1");
    logger.debug("Message 2");
    logger.error("Message 3");
    
    auto buffer = logger.buffer();
    UTEST_ASSERT_NOT_EQUALS(buffer, nullptr);
    UTEST_ASSERT_EQUALS(buffer->size(), 3);
    UTEST_ASSERT_FALSE(buffer->empty());
}

UTEST_FUNC_DEF2(Buffer, Capacity) {
    auto& logger = ulog::getLogger("CapacityTest");
    logger.disable_console();
    
    logger.enable_buffer(2); // Capacity of 2
    
    logger.info("Message 1");
    logger.info("Message 2");
    logger.info("Message 3"); // Should evict first message
    
    auto buffer = logger.buffer();
    UTEST_ASSERT_EQUALS(buffer->size(), 2);
}

UTEST_FUNC_DEF2(Buffer, Clear) {
    auto& logger = ulog::getLogger("ClearTest");
    logger.disable_console();
    
    logger.enable_buffer(100);
    
    logger.info("Message 1");
    logger.info("Message 2");
    
    auto buffer = logger.buffer();
    UTEST_ASSERT_EQUALS(buffer->size(), 2);
    
    logger.clear_buffer();
    UTEST_ASSERT_EQUALS(buffer->size(), 0);
    UTEST_ASSERT_TRUE(buffer->empty());
}

UTEST_FUNC_DEF2(Buffer, Iteration) {
    auto& logger = ulog::getLogger("IterationTest");
    logger.disable_console();
    
    logger.enable_buffer(100);
    
    logger.info("First");
    logger.debug("Second");
    logger.warn("Third");
    
    auto buffer = logger.buffer();
    UTEST_ASSERT_EQUALS(buffer->size(), 3);
    
    std::vector<std::string> messages;
    for (auto it = buffer->cbegin(); it != buffer->cend(); ++it) {
        messages.push_back(it->message);
    }
    
    UTEST_ASSERT_EQUALS(messages.size(), 3);
    UTEST_ASSERT_EQUALS(messages[0], "First");
    UTEST_ASSERT_EQUALS(messages[1], "Second");
    UTEST_ASSERT_EQUALS(messages[2], "Third");
}

UTEST_FUNC_DEF2(Buffer, ThreadSafety) {
    auto& logger = ulog::getLogger("BufferThreadTest");
    logger.disable_console();
    logger.enable_buffer(0); // Unlimited
    
    const int num_threads = 10;
    const int messages_per_thread = 50;
    
    std::vector<std::thread> threads;
    
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
    
    auto buffer = logger.buffer();
    UTEST_ASSERT_EQUALS(buffer->size(), num_threads * messages_per_thread);
}

void test_buffer_register(bool& errorFound) {
    UTEST_FUNC2(Buffer, EnableDisable);
    UTEST_FUNC2(Buffer, Storage);
    UTEST_FUNC2(Buffer, Capacity);
    UTEST_FUNC2(Buffer, Clear);
    UTEST_FUNC2(Buffer, Iteration);
    UTEST_FUNC2(Buffer, ThreadSafety);
}

int main() {
    UTEST_PROLOG();
    
    test_buffer_register(errorFound);
    
    UTEST_EPILOG();
    
    return 0;
}
