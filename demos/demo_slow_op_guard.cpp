/**
 * @file demo_slow_op_guard.cpp
 * @brief Demonstration of SlowOpGuard RAII functionality for monitoring slow operations
 * 
 * This demo showcases the SlowOpGuard extension that automatically monitors
 * operation duration and logs warnings when operations exceed specified time limits.
 * It includes examples with both static messages and lambda-based message suppliers,
 * along with a fake database stub for realistic testing scenarios.
 */

#include "ulog/ulog.h"
#include "extensions/include/slow_op_guard.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <random>

using ulog::extensions::SlowOpGuard;

/**
 * @brief Simple database stub for demo purposes
 */
class DatabaseStub {
private:
    std::mt19937 rng_;
    std::uniform_int_distribution<int> delay_dist_;
    
public:
    DatabaseStub() : rng_(std::random_device{}()), delay_dist_(10, 200) {}
    
    /**
     * @brief Simulates a database query with variable delay
     * @param query_name Name of the query for logging
     * @return Simulated result count
     */
    int execute_query(const std::string& query_name) {
        // Simulate variable query execution time
        int delay_ms = delay_dist_(rng_);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        
        std::cout << "  [DB] Executed query '" << query_name 
                  << "' (simulated delay: " << delay_ms << "ms)" << std::endl;
        
        // Return simulated result count
        return delay_ms % 10 + 1;
    }
    
    /**
     * @brief Simulates a slow database operation
     * @param operation_name Name of the operation
     */
    void execute_slow_operation(const std::string& operation_name) {
        // Intentionally slow operation (150-300ms)
        int delay_ms = 150 + (delay_dist_(rng_) % 150);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        
        std::cout << "  [DB] Completed slow operation '" << operation_name 
                  << "' (simulated delay: " << delay_ms << "ms)" << std::endl;
    }
    
    /**
     * @brief Simulates a fast database operation
     * @param operation_name Name of the operation
     */
    void execute_fast_operation(const std::string& operation_name) {
        // Fast operation (5-25ms)
        int delay_ms = 5 + (delay_dist_(rng_) % 20);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        
        std::cout << "  [DB] Completed fast operation '" << operation_name 
                  << "' (simulated delay: " << delay_ms << "ms)" << std::endl;
    }
};

/**
 * @brief Demo function showing basic SlowOpGuard usage with static messages
 */
void demo_basic_slow_op_guard() {
    std::cout << "\n=== DEMO: Basic SlowOpGuard with Static Messages ===" << std::endl;
    
    auto& logger = ulog::getLogger("SlowOpDemo");
    logger.set_log_level(ulog::LogLevel::TRACE);
    
    DatabaseStub db;
    
    std::cout << "\n1. Fast operation (should not trigger warning):" << std::endl;
    {
        SlowOpGuard guard(logger, std::chrono::milliseconds(100), 
                         ulog::LogLevel::WARN, "SELECT * FROM users WHERE active = 1");
        db.execute_fast_operation("select_active_users");
    }
    
    std::cout << "\n2. Slow operation (should trigger warning):" << std::endl;
    {
        SlowOpGuard guard(logger, std::chrono::milliseconds(100), 
                         ulog::LogLevel::WARN, "SELECT * FROM orders JOIN customers ON orders.customer_id = customers.id");
        db.execute_slow_operation("complex_join_query");
    }
    
    std::cout << "\n3. Different log levels:" << std::endl;
    {
        SlowOpGuard guard(logger, std::chrono::milliseconds(50), 
                         ulog::LogLevel::ERROR, "UPDATE products SET stock = stock - 1");
        db.execute_slow_operation("update_stock");
    }
}

/**
 * @brief Demo function showing SlowOpGuard with lambda message suppliers
 */
void demo_lambda_message_suppliers() {
    std::cout << "\n=== DEMO: SlowOpGuard with Lambda Message Suppliers ===" << std::endl;
    
    auto& logger = ulog::getLogger("SlowOpDemo");
    DatabaseStub db;
    
    std::cout << "\n1. Lambda with elapsed time formatting:" << std::endl;
    {
        SlowOpGuard guard(logger, std::chrono::milliseconds(80), 
                         ulog::LogLevel::WARN, 
                         [](auto elapsed) {
                             return "Database backup operation took " + 
                                    std::to_string(elapsed.count()) + 
                                    "ms (threshold exceeded!)";
                         });
        db.execute_slow_operation("database_backup");
    }
    
    std::cout << "\n2. Lambda with performance classification:" << std::endl;
    {
        SlowOpGuard guard(logger, std::chrono::milliseconds(100), 
                         ulog::LogLevel::ERROR, 
                         [](auto elapsed) {
                             std::string performance_level;
                             if (elapsed.count() > 200) {
                                 performance_level = "CRITICAL";
                             } else if (elapsed.count() > 150) {
                                 performance_level = "POOR";
                             } else {
                                 performance_level = "ACCEPTABLE";
                             }
                             return "Query performance: " + performance_level + 
                                    " (" + std::to_string(elapsed.count()) + "ms)";
                         });
        db.execute_slow_operation("performance_sensitive_query");
    }
    
    std::cout << "\n3. Lambda with contextual information:" << std::endl;
    {
        std::string user_id = "user_12345";
        std::string session_id = "sess_abcdef";
        
        SlowOpGuard guard(logger, std::chrono::milliseconds(75), 
                         ulog::LogLevel::WARN, 
                         [user_id, session_id](auto elapsed) {
                             return "Slow user operation detected - User: " + user_id + 
                                    ", Session: " + session_id + 
                                    ", Duration: " + std::to_string(elapsed.count()) + "ms";
                         });
        db.execute_slow_operation("user_profile_update");
    }
}

/**
 * @brief Demo function showing multiple guards in nested scenarios
 */
void demo_nested_operations() {
    std::cout << "\n=== DEMO: Nested SlowOpGuard Operations ===" << std::endl;
    
    auto& logger = ulog::getLogger("SlowOpDemo");
    DatabaseStub db;
    
    std::cout << "\nProcessing batch operation with nested guards:" << std::endl;
    
    {
        SlowOpGuard batch_guard(logger, std::chrono::milliseconds(400), 
                               ulog::LogLevel::ERROR, "entire batch processing");
        
        logger.info("Starting batch processing...");
        
        for (int i = 1; i <= 3; ++i) {
            SlowOpGuard item_guard(logger, std::chrono::milliseconds(120), 
                                  ulog::LogLevel::WARN, 
                                  [i](auto elapsed) {
                                      return "Batch item " + std::to_string(i) + 
                                             " processing took " + std::to_string(elapsed.count()) + "ms";
                                  });
            
            std::cout << "  Processing item " << i << "..." << std::endl;
            if (i == 2) {
                db.execute_slow_operation("complex_item_processing");
            } else {
                db.execute_fast_operation("simple_item_processing");
            }
        }
        
        logger.info("Batch processing completed");
    }
}

/**
 * @brief Demo function showing real-world scenarios
 */
void demo_real_world_scenarios() {
    std::cout << "\n=== DEMO: Real-World SlowOpGuard Scenarios ===" << std::endl;
    
    auto& logger = ulog::getLogger("SlowOpDemo");
    DatabaseStub db;
    
    // Scenario 1: API endpoint monitoring
    std::cout << "\n1. API Endpoint Monitoring:" << std::endl;
    {
        SlowOpGuard api_guard(logger, std::chrono::milliseconds(200), 
                             ulog::LogLevel::WARN, "/api/v1/users/{id}/orders endpoint");
        
        logger.info("Processing API request...");
        db.execute_query("user_orders_with_details");
        logger.info("API request completed");
    }
    
    // Scenario 2: File processing with dynamic thresholds
    std::cout << "\n2. File Processing with Dynamic Thresholds:" << std::endl;
    std::vector<std::pair<std::string, int>> files = {
        {"small_file.csv", 50},
        {"medium_file.csv", 100},
        {"large_file.csv", 200}
    };
    
    for (const auto& file_info : files) {
        const std::string& filename = file_info.first;
        int threshold_ms = file_info.second;
        
        SlowOpGuard file_guard(logger, std::chrono::milliseconds(threshold_ms), 
                              ulog::LogLevel::WARN, 
                              [filename](auto elapsed) {
                                  return "File processing slow: " + filename + 
                                         " took " + std::to_string(elapsed.count()) + "ms";
                              });
        
        std::cout << "  Processing " << filename << " (threshold: " << threshold_ms << "ms)..." << std::endl;
        if (filename.find("large") != std::string::npos) {
            db.execute_slow_operation("large_file_processing");
        } else {
            db.execute_fast_operation("regular_file_processing");
        }
    }
    
    // Scenario 3: Transaction monitoring
    std::cout << "\n3. Database Transaction Monitoring:" << std::endl;
    {
        SlowOpGuard transaction_guard(logger, std::chrono::milliseconds(150), 
                                     ulog::LogLevel::ERROR, 
                                     [](auto elapsed) {
                                         if (elapsed.count() > 300) {
                                             return "CRITICAL: Transaction deadlock suspected! Duration: " + 
                                                    std::to_string(elapsed.count()) + "ms";
                                         } else {
                                             return "Transaction performance warning: " + 
                                                    std::to_string(elapsed.count()) + "ms";
                                         }
                                     });
        
        logger.info("Beginning database transaction...");
        db.execute_slow_operation("transaction_with_multiple_updates");
        logger.info("Transaction committed");
    }
}

/**
 * @brief Demo function showing guard introspection capabilities
 */
void demo_guard_introspection() {
    std::cout << "\n=== DEMO: SlowOpGuard Introspection ===" << std::endl;
    
    auto& logger = ulog::getLogger("SlowOpDemo");
    DatabaseStub db;
    
    std::cout << "\nMonitoring operation with introspection:" << std::endl;
    
    SlowOpGuard guard(logger, std::chrono::milliseconds(100), 
                     ulog::LogLevel::WARN, "monitored operation");
    
    logger.info("Operation started (limit: {0}ms)", guard.get_time_limit().count());
    
    // Simulate checking progress during operation
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        
        auto current_elapsed = guard.elapsed();
        bool is_slow = guard.is_slow();
        
        std::cout << "  Progress check " << (i + 1) << ": " 
                  << current_elapsed.count() << "ms elapsed"
                  << (is_slow ? " (SLOW!)" : " (OK)") << std::endl;
        
        if (is_slow) {
            logger.warn("Operation is running slow: {0}ms elapsed", current_elapsed.count());
        }
    }
    
    // Guard will automatically log if total time exceeds threshold when destroyed
}

int main() {
    std::cout << "SlowOpGuard Extension Demo" << std::endl;
    std::cout << "=========================" << std::endl;
    
    try {
        demo_basic_slow_op_guard();
        demo_lambda_message_suppliers();
        demo_nested_operations();
        demo_real_world_scenarios();
        demo_guard_introspection();
        
        std::cout << "\n=== Demo completed successfully ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
