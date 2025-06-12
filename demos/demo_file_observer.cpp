#include "ulog/ulog.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

/**
 * @brief File observer that writes log entries to a file
 */
class FileObserver : public ulog::LogObserver {
public:
    /**
     * @brief Constructor
     * @param filename Path to the log file
     * @param append If true, append to existing file; if false, overwrite
     */
    explicit FileObserver(const std::string& filename, bool append = true) 
        : filename_(filename), append_(append) {
        // Open file in appropriate mode
        auto mode = append ? std::ios::app : std::ios::out;
        file_.open(filename_, mode);
        
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename_);
        }
        
        std::cout << "[FILE_OBSERVER] Created file observer for: " << filename_ << std::endl;
    }
    
    /**
     * @brief Destructor - ensures file is properly closed
     */
    ~FileObserver() {
        if (file_.is_open()) {
            file_.close();
        }
    }
    
    void handleRegistered(const std::string& logger_name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open()) {
            file_ << "# Observer registered to logger: " << logger_name << std::endl;
            file_.flush();
        }
        std::cout << "[FILE_OBSERVER] Registered to logger: " << logger_name << std::endl;
    }
    
    void handleUnregistered(const std::string& logger_name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open()) {
            file_ << "# Observer unregistered from logger: " << logger_name << std::endl;
            file_.flush();
        }
        std::cout << "[FILE_OBSERVER] Unregistered from logger: " << logger_name << std::endl;
    }
    
    void handleNewMessage(const ulog::LogEntry& entry) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open()) {
            // Write the formatted log message to file
            file_ << entry.formatted_message() << std::endl;
            file_.flush(); // Ensure immediate write
        }
    }
    
    void handleFlush(const std::string& logger_name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open()) {
            file_ << "# Flush requested for logger: " << logger_name << std::endl;
            file_.flush();
        }
        std::cout << "[FILE_OBSERVER] Flushed logger: " << logger_name << std::endl;
    }
    
    /**
     * @brief Get the filename being written to
     * @return The log file path
     */
    const std::string& getFilename() const {
        return filename_;
    }
    
    /**
     * @brief Check if file is currently open and writable
     * @return True if file is open
     */
    bool isOpen() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return file_.is_open();
    }

private:
    std::string filename_;
    bool append_;
    std::ofstream file_;
    mutable std::mutex mutex_;
};

/**
 * @brief Demo function showcasing file output via observer
 */
void demo_file_output_observer() {
    std::cout << "\n=== DEMO: File Output via Observer ===" << std::endl;
    
    // Create a logger
    auto& logger = ulog::getLogger("FileDemo");
    
    // Disable console output to avoid duplication in this demo
    logger.disable_console();
    
    try {
        // Create file observer that writes to "demo_log.txt"
        auto fileObserver = std::make_shared<FileObserver>("demo_log.txt", false); // overwrite mode
        
        std::cout << "Created file observer writing to: " << fileObserver->getFilename() << std::endl;
        
        // Manual observer management
        logger.add_observer(fileObserver);
        
        // Log various messages at different levels
        logger.info("Application started - this should appear in the file");
        logger.debug("Debug information: initializing components");
        logger.warn("Warning: configuration file not found, using defaults");
        logger.error("Error: failed to connect to database");
        logger.info("Retrying database connection...");
        logger.info("Successfully connected to database");
        
        // Manually flush to ensure all data is written
        logger.flush();
        
        // Remove observer
        logger.remove_observer(fileObserver);
        
        // This message won't be written to file since observer is removed
        logger.enable_console(); // Enable console just for this message
        logger.info("This message appears only on console (not in file)");
        logger.disable_console();
        
        std::cout << "Demo completed. Check 'demo_log.txt' for the logged messages." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Demo function showcasing RAII file observer management
 */
void demo_raii_file_observer() {
    std::cout << "\n=== DEMO: RAII File Observer Management ===" << std::endl;
    
    auto& logger = ulog::getLogger("RAIIDemo");
    logger.disable_console();
    
    try {
        // Create file observer for append mode
        auto fileObserver = std::make_shared<FileObserver>("demo_log_raii.txt", true); // append mode
        
        std::cout << "Created RAII file observer writing to: " << fileObserver->getFilename() << std::endl;
        
        // RAII observer management
        {
            ulog::ObserverScope scope(logger, fileObserver);
            
            logger.info("RAII scope started");
            logger.debug("Processing within RAII scope");
            logger.warn("Warning within RAII scope");
            logger.info("RAII scope ending...");
            
        } // Observer automatically removed here
        
        // This won't be written to file
        logger.enable_console();
        logger.info("Message after RAII scope - not in file");
        
        std::cout << "RAII demo completed. Check 'demo_log_raii.txt' for the logged messages." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Demo function showcasing multiple file observers
 */
void demo_multiple_file_observers() {
    std::cout << "\n=== DEMO: Multiple File Observers ===" << std::endl;
    
    auto& logger = ulog::getLogger("MultiFileDemo");
    logger.disable_console();
    
    try {
        // Create multiple file observers
        auto generalLog = std::make_shared<FileObserver>("demo_general.log", false);
        auto errorLog = std::make_shared<FileObserver>("demo_errors.log", false);
        
        // Custom observer that only logs error and fatal messages
        class ErrorOnlyFileObserver : public FileObserver {
        public:
            explicit ErrorOnlyFileObserver(const std::string& filename, bool append = true) 
                : FileObserver(filename, append) {}
            
            void handleNewMessage(const ulog::LogEntry& entry) override {
                // Only write ERROR and FATAL messages to this file
                if (entry.level == ulog::LogLevel::ERROR || entry.level == ulog::LogLevel::FATAL) {
                    FileObserver::handleNewMessage(entry);
                }
            }
        };
        
        auto errorOnlyObserver = std::make_shared<ErrorOnlyFileObserver>("demo_errors_only.log", false);
        
        // Add all observers
        logger.add_observer(generalLog);
        logger.add_observer(errorLog);
        logger.add_observer(errorOnlyObserver);
        
        std::cout << "Added multiple file observers:" << std::endl;
        std::cout << "  - General log: " << generalLog->getFilename() << std::endl;
        std::cout << "  - Error log: " << errorLog->getFilename() << std::endl;
        std::cout << "  - Error-only log: " << errorOnlyObserver->getFilename() << std::endl;
        
        // Log messages at different levels
        logger.info("System initialization started");
        logger.debug("Loading configuration...");
        logger.info("Configuration loaded successfully");
        logger.warn("Deprecated API usage detected");
        logger.error("Failed to validate user input");
        logger.info("Attempting recovery...");
        logger.fatal("Critical system failure");
        logger.info("System shutdown initiated");
        
        // Flush and cleanup
        logger.flush();
        logger.remove_observer(generalLog);
        logger.remove_observer(errorLog);
        logger.remove_observer(errorOnlyObserver);
        
        std::cout << "Multiple file observers demo completed." << std::endl;
        std::cout << "Check the following files:" << std::endl;
        std::cout << "  - demo_general.log (all messages)" << std::endl;
        std::cout << "  - demo_errors.log (all messages)" << std::endl;
        std::cout << "  - demo_errors_only.log (only ERROR and FATAL messages)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Main demo function
 */
int main() {
    std::cout << "=== ulog File Observer Demo ===" << std::endl;
    std::cout << "This demo showcases file output functionality using the observer pattern." << std::endl;
    
    // Run all file observer demos
    demo_file_output_observer();
    demo_raii_file_observer();
    demo_multiple_file_observers();
    
    std::cout << "\n=== Demo Summary ===" << std::endl;
    std::cout << "The file observer demo has created several log files:" << std::endl;
    std::cout << "1. demo_log.txt - Basic file logging demo" << std::endl;
    std::cout << "2. demo_log_raii.txt - RAII observer management demo" << std::endl;
    std::cout << "3. demo_general.log - General logging (multiple observers demo)" << std::endl;
    std::cout << "4. demo_errors.log - General logging copy" << std::endl;
    std::cout << "5. demo_errors_only.log - Filtered error messages only" << std::endl;
    std::cout << "\nYou can examine these files to see the file output functionality in action." << std::endl;
    
    return 0;
}
