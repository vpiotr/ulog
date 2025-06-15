#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <memory>

// Enable all mutexes for baseline comparison
#define ULOG_USE_MUTEX_FOR_CONSOLE 1
#define ULOG_USE_MUTEX_FOR_BUFFER 1
#define ULOG_USE_MUTEX_FOR_OBSERVERS 1
#include "ulog/ulog.h"

/**
 * @brief Simple observer that counts messages
 */
class CountingObserver : public ulog::LogObserver {
public:
    void handleNewMessage(const ulog::LogEntry& entry) override {
        message_count_++;
    }
    
    void handleRegistered(const std::string& logger_name) override {
        // No-op for benchmark
    }
    
    void handleUnregistered(const std::string& logger_name) override {
        // No-op for benchmark
    }
    
    void handleFlush(const std::string& logger_name) override {
        // No-op for benchmark
    }
    
    size_t getMessageCount() const { return message_count_; }
    void reset() { message_count_ = 0; }
    
private:
    std::atomic<size_t> message_count_{0};
};

int main() {
    std::cout << "=== Observer Performance Benchmark - WITH OBSERVER MUTEX (Single Thread) ===" << std::endl;
    std::cout << "ULOG_USE_MUTEX_FOR_OBSERVERS=1" << std::endl;
    std::cout << "Testing single-threaded observer notification performance with mutex protection" << std::endl;
    std::cout << std::endl;
    
    auto& logger = ulog::getLogger("ObserverBenchLogger");
    
    // Disable console and buffer to focus purely on observer performance
    logger.disable_console();
    logger.disable_buffer();
    
    // Add multiple observers to simulate real-world usage
    auto observer1 = std::make_shared<CountingObserver>();
    auto observer2 = std::make_shared<CountingObserver>();
    auto observer3 = std::make_shared<CountingObserver>();
    
    logger.add_observer(observer1);
    logger.add_observer(observer2);
    logger.add_observer(observer3);
    
    const int num_messages = 100000;
    std::vector<double> iteration_times;
    iteration_times.reserve(5);
    
    std::cout << "Number of observers: 3" << std::endl;
    std::cout << "Messages per iteration: " << num_messages << std::endl;
    std::cout << "Running 5 iterations..." << std::endl;
    std::cout << std::endl;
    
    for (int iteration = 0; iteration < 5; ++iteration) {
        observer1->reset();
        observer2->reset();
        observer3->reset();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Write messages that will trigger observer notifications
        for (int i = 0; i < num_messages; ++i) {
            logger.info("Observer benchmark message {0} iteration {1}", i, iteration);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double duration_ms = duration.count() / 1000.0;
        iteration_times.push_back(duration_ms);
        
        std::cout << "Iteration " << (iteration + 1) << ": " 
                  << duration_ms << " ms (" 
                  << (num_messages / duration_ms * 1000.0) << " msg/sec)" << std::endl;
                  
        // Verify observer counts
        std::cout << "  Observer counts: " << observer1->getMessageCount() 
                  << ", " << observer2->getMessageCount() 
                  << ", " << observer3->getMessageCount() << std::endl;
    }
    
    // Calculate statistics
    double total_time = 0.0;
    double min_time = iteration_times[0];
    double max_time = iteration_times[0];
    
    for (double time : iteration_times) {
        total_time += time;
        min_time = std::min(min_time, time);
        max_time = std::max(max_time, time);
    }
    
    double avg_time = total_time / iteration_times.size();
    double avg_throughput = num_messages / avg_time * 1000.0;
    
    std::cout << std::endl;
    std::cout << "=== RESULTS ===" << std::endl;
    std::cout << "Average time: " << avg_time << " ms" << std::endl;
    std::cout << "Min time: " << min_time << " ms" << std::endl;
    std::cout << "Max time: " << max_time << " ms" << std::endl;
    std::cout << "Average throughput: " << static_cast<int>(avg_throughput) << " messages/second" << std::endl;
    std::cout << "Total observer notifications: " << (num_messages * 3) << " per iteration" << std::endl;
    
    // Clean up observers
    logger.remove_observer(observer3);
    logger.remove_observer(observer2);
    logger.remove_observer(observer1);
    
    std::cout << std::endl;
    std::cout << "Benchmark completed successfully!" << std::endl;
    std::cout << "Note: This benchmark ran WITH observer mutex protection." << std::endl;
    std::cout << "Compare with demo_observer_benchmark_no_mutex to see the performance difference." << std::endl;
    
    return 0;
}
