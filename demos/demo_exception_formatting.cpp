/**
 * @file demo_exception_formatting.cpp
 * @brief Demonstration of auto-formatting exceptions in ulog
 * 
 * This demo showcases how to automatically format std::exception and derived
 * exception types using custom formatting, enabling rich exception logging
 * with detailed information extraction.
 */

#include "ulog/ulog.h"
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <filesystem>
#include <typeinfo>
#include <memory>
#include <vector>

/**
 * @brief Custom exception wrapper for enhanced formatting
 */
class ExceptionFormatter {
private:
    const std::exception* exception_;
    bool show_type_;
    bool show_what_;
    bool show_nested_;

public:
    ExceptionFormatter(const std::exception& ex, bool show_type = true, 
                      bool show_what = true, bool show_nested = true)
        : exception_(&ex), show_type_(show_type), show_what_(show_what), show_nested_(show_nested) {}

    friend std::ostream& operator<<(std::ostream& os, const ExceptionFormatter& ef) {
        if (ef.show_type_) {
            // Get clean type name without namespace prefixes
            std::string type_name = typeid(*ef.exception_).name();
            if (type_name.find("std::") == 0) {
                type_name = type_name.substr(5); // Remove "std::" prefix
            }
            os << "[" << type_name << "]";
        }
        
        if (ef.show_what_ && ef.exception_->what()) {
            if (ef.show_type_) os << " ";
            os << ef.exception_->what();
        }

        // Check for nested exceptions
        if (ef.show_nested_) {
            try {
                std::rethrow_if_nested(*ef.exception_);
            } catch (const std::exception& nested) {
                os << " (caused by: " << ExceptionFormatter(nested, true, true, false) << ")";
            } catch (...) {
                os << " (caused by: unknown exception)";
            }
        }
        
        return os;
    }
};

/**
 * @brief Helper function to create formatted exception wrapper
 */
ExceptionFormatter format_exception(const std::exception& ex, bool show_type = true, 
                                   bool show_what = true, bool show_nested = true) {
    return ExceptionFormatter(ex, show_type, show_what, show_nested);
}

/**
 * @brief Custom exception class for demonstration
 */
class DatabaseError : public std::runtime_error {
private:
    int error_code_;
    std::string connection_string_;

public:
    DatabaseError(const std::string& message, int code, const std::string& conn_str)
        : std::runtime_error(message), error_code_(code), connection_string_(conn_str) {}

    int getErrorCode() const { return error_code_; }
    const std::string& getConnectionString() const { return connection_string_; }
};

/**
 * @brief Custom formatting for DatabaseError
 */
std::ostream& operator<<(std::ostream& os, const DatabaseError& db_err) {
    os << "[DatabaseError:" << db_err.getErrorCode() << "] " 
       << db_err.what() << " (connection: " << db_err.getConnectionString() << ")";
    return os;
}

/**
 * @brief Network exception with additional context
 */
class NetworkError : public std::runtime_error {
private:
    std::string host_;
    int port_;
    int timeout_ms_;

public:
    NetworkError(const std::string& message, const std::string& host, int port, int timeout = 5000)
        : std::runtime_error(message), host_(host), port_(port), timeout_ms_(timeout) {}

    const std::string& getHost() const { return host_; }
    int getPort() const { return port_; }
    int getTimeout() const { return timeout_ms_; }
};

// Custom ustr::to_string specialization for NetworkError
namespace ulog {
namespace ustr {
    template<>
    std::string to_string<NetworkError>(const NetworkError& net_err) {
        std::ostringstream ss;
        ss << "[NetworkError] " << net_err.what() 
           << " (host: " << net_err.getHost() 
           << ":" << net_err.getPort() 
           << ", timeout: " << net_err.getTimeout() << "ms)";
        return ss.str();
    }
}
}

/**
 * @brief Demo function showcasing basic exception formatting
 */
void demo_basic_exception_formatting() {
    std::cout << "\n=== DEMO: Basic Exception Formatting ===" << std::endl;
    
    auto& logger = ulog::getLogger("ExceptionDemo");
    logger.set_log_level(ulog::LogLevel::DEBUG);

    try {
        // Standard exceptions
        std::runtime_error runtime_err("Runtime operation failed");
        std::invalid_argument invalid_arg("Invalid parameter provided");
        std::out_of_range out_of_range("Index out of bounds");

        logger.info("Logging standard exceptions:");
        logger.error("Runtime error: {?}", format_exception(runtime_err));
        logger.error("Invalid argument: {?}", format_exception(invalid_arg));
        logger.error("Out of range: {?}", format_exception(out_of_range));

        // Using different formatting options
        logger.debug("Exception with type only: {?}", format_exception(runtime_err, true, false, false));
        logger.debug("Exception with message only: {?}", format_exception(runtime_err, false, true, false));
        
    } catch (const std::exception& e) {
        logger.fatal("Demo error: {?}", format_exception(e));
    }
}

/**
 * @brief Demo function showcasing custom exception formatting
 */
void demo_custom_exception_formatting() {
    std::cout << "\n=== DEMO: Custom Exception Formatting ===" << std::endl;
    
    auto& logger = ulog::getLogger("CustomExceptionDemo");

    try {
        // Custom exceptions with rich formatting
        DatabaseError db_err("Connection timeout", 1001, "postgresql://localhost:5432/mydb");
        NetworkError net_err("Connection refused", "api.example.com", 443, 10000);

        logger.error("Database operation failed: {?}", db_err);
        logger.error("Network request failed: {?}", net_err);

        // Demonstrate error handling in operations
        logger.info("Attempting database connection...");
        logger.warn("First attempt failed: {?}", db_err);
        logger.info("Retrying with different parameters...");
        logger.error("Second attempt failed: {?}", net_err);

    } catch (const std::exception& e) {
        logger.fatal("Demo error: {?}", format_exception(e));
    }
}

/**
 * @brief Function that creates nested exceptions for demonstration
 */
void throw_nested_exception() {
    try {
        NetworkError net_err("Connection refused", "backend.service", 8080);
        throw net_err;
    } catch (...) {
        std::throw_with_nested(DatabaseError("Transaction failed due to network issue", 5001, "tcp://backend:5432"));
    }
}

/**
 * @brief Demo function showcasing nested exception formatting
 */
void demo_nested_exception_formatting() {
    std::cout << "\n=== DEMO: Nested Exception Formatting ===" << std::endl;
    
    auto& logger = ulog::getLogger("NestedExceptionDemo");

    try {
        logger.info("Starting complex operation with multiple failure points...");
        throw_nested_exception();
    } catch (const std::exception& e) {
        logger.error("Operation failed with nested cause: {?}", format_exception(e));
        logger.debug("Exception details: {?}", format_exception(e, true, true, true));
        
        // Log each level separately for detailed analysis
        const std::exception* current = &e;
        int level = 0;
        while (current) {
            logger.debug("Exception level {?}: {?}", level++, format_exception(*current, true, true, false));
            try {
                std::rethrow_if_nested(*current);
                break;
            } catch (const std::exception& nested) {
                current = &nested;
            } catch (...) {
                logger.debug("Unknown nested exception at level {?}", level);
                break;
            }
        }
    }
}

/**
 * @brief Demo function showcasing system error formatting
 */
void demo_system_error_formatting() {
    std::cout << "\n=== DEMO: System Error Formatting ===" << std::endl;
    
    auto& logger = ulog::getLogger("SystemErrorDemo");

    try {
        // Simulate system errors
        std::error_code ec = std::make_error_code(std::errc::permission_denied);
        std::system_error sys_err(ec, "Failed to access file");
        
        logger.error("System error occurred: {?}", format_exception(sys_err));
        
        // Create filesystem error (if available)
        try {
            std::filesystem::path invalid_path("/root/secret_file.txt");
            // This would normally throw, but we'll simulate it
            std::filesystem::filesystem_error fs_err("Access denied", invalid_path, ec);
            logger.error("Filesystem error: {?}", format_exception(fs_err));
        } catch (...) {
            logger.debug("Filesystem error simulation skipped");
        }

    } catch (const std::exception& e) {
        logger.fatal("Demo error: {?}", format_exception(e));
    }
}

/**
 * @brief Demo function showcasing exception logging in real scenarios
 */
void demo_real_world_exception_scenarios() {
    std::cout << "\n=== DEMO: Real-World Exception Scenarios ===" << std::endl;
    
    auto& logger = ulog::getLogger("RealWorldDemo");

    // Scenario 1: Resource initialization failure
    try {
        logger.info("Initializing application resources...");
        throw DatabaseError("Connection pool exhausted", 2001, "mysql://prod-db:3306/app");
    } catch (const DatabaseError& e) {
        logger.error("Resource initialization failed: {?}", e);
        logger.warn("Falling back to read-only mode");
    }

    // Scenario 2: API call failure with retries
    for (int attempt = 1; attempt <= 3; ++attempt) {
        try {
            logger.debug("API call attempt {?}/3", attempt);
            if (attempt < 3) {
                throw NetworkError("Service temporarily unavailable", "api.service.com", 443, 5000);
            }
            logger.info("API call succeeded on attempt {?}", attempt);
            break;
        } catch (const NetworkError& e) {
            if (attempt < 3) {
                logger.warn("Attempt {?} failed: {?}, retrying...", attempt, e);
            } else {
                logger.error("All attempts failed: {?}", e);
            }
        }
    }

    // Scenario 3: Data processing error with context
    std::vector<std::string> data_files = {"user_data.csv", "invalid_file.json", "config.xml"};
    
    for (size_t i = 0; i < data_files.size(); ++i) {
        try {
            logger.debug("Processing file {?}/{?}: {?}", i + 1, data_files.size(), data_files[i]);
            
            if (data_files[i] == "invalid_file.json") {
                throw std::invalid_argument("Malformed JSON in file");
            }
            
            logger.trace("File {?} processed successfully", data_files[i]);
            
        } catch (const std::exception& e) {
            logger.error("Failed to process file {?}: {?}", data_files[i], format_exception(e));
            logger.info("Continuing with next file...");
        }
    }
}

/**
 * @brief Demo function showcasing exception formatting performance considerations
 */
void demo_exception_formatting_performance() {
    std::cout << "\n=== DEMO: Exception Formatting Performance ===" << std::endl;
    
    auto& logger = ulog::getLogger("PerfDemo");

    // For performance-critical logging, check log level first
    DatabaseError heavy_exception("Complex database operation failed with detailed context", 4001, 
                                 "postgresql://master.db.cluster.internal:5432/analytics_warehouse");

    // Efficient: only format if error logging is enabled
    if (logger.get_log_level() <= ulog::LogLevel::ERROR) {
        logger.error("Performance-conscious exception logging: {?}", heavy_exception);
    }

    // Less efficient: always creates formatter
    logger.debug("This debug message might not appear: {?}", format_exception(heavy_exception));

    // Best practice: combine with log level check for expensive formatting
    if (logger.get_log_level() <= ulog::LogLevel::DEBUG) {
        // Only do expensive formatting if debug is enabled
        logger.debug("Detailed exception analysis: {?}", format_exception(heavy_exception, true, true, true));
    }

    logger.info("Performance demo completed");
}

/**
 * @brief Main demo function
 */
int main() {
    std::cout << "=== ulog Exception Formatting Demo ===" << std::endl;
    std::cout << "This demo showcases automatic formatting of exceptions for enhanced error logging." << std::endl;
    
    try {
        demo_basic_exception_formatting();
        demo_custom_exception_formatting();
        demo_nested_exception_formatting();
        demo_system_error_formatting();
        demo_real_world_exception_scenarios();
        demo_exception_formatting_performance();
        
        std::cout << "\n=== Demo completed successfully! ===" << std::endl;
        std::cout << "\nKey Takeaways:" << std::endl;
        std::cout << "1. Use ExceptionFormatter wrapper for automatic exception formatting" << std::endl;
        std::cout << "2. Implement operator<< for custom exception types" << std::endl;
        std::cout << "3. Specialize ulog::ustr::to_string for advanced custom formatting" << std::endl;
        std::cout << "4. Handle nested exceptions to show complete error chains" << std::endl;
        std::cout << "5. Check log levels before expensive exception formatting" << std::endl;
        std::cout << "6. Include relevant context (error codes, connection strings, etc.)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Demo failed with exception: " << format_exception(e) << std::endl;
        return 1;
    }
    
    return 0;
}
