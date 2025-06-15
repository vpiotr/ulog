#include <iostream>
#include <chrono>
#include <string>
#include <vector>

// Enable mutex for buffer operations (default)
#define ULOG_USE_MUTEX_FOR_BUFFER 1
// Enable mutex for observer operations (default)
#define ULOG_USE_MUTEX_FOR_OBSERVERS 1
// Keep console mutex enabled (default)
#include "ulog/ulog.h"

int main() {
    std::cout << "=== Buffer Write Benchmark - WITH BUFFER/OBSERVER MUTEX (Single Thread) ===" << std::endl;
    std::cout << "ULOG_USE_MUTEX_FOR_BUFFER=1, ULOG_USE_MUTEX_FOR_OBSERVERS=1" << std::endl;
    std::cout << "Testing single-threaded buffer write performance with mutex protection" << std::endl;
    std::cout << std::endl;
    
    auto& logger = ulog::getLogger("BenchmarkLogger");
    
    // Disable console output to focus on buffer performance
    logger.disable_console();
    
    // Enable buffer with large capacity
    const size_t buffer_capacity = 100000;
    logger.enable_buffer(buffer_capacity);
    
    const int num_messages = 50000;
    std::vector<double> iteration_times;
    iteration_times.reserve(5);
    
    std::cout << "Buffer capacity: " << buffer_capacity << std::endl;
    std::cout << "Messages per iteration: " << num_messages << std::endl;
    std::cout << "Running 5 iterations..." << std::endl;
    std::cout << std::endl;
    
    for (int iteration = 0; iteration < 5; ++iteration) {
        logger.clear_buffer();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Write messages to buffer
        for (int i = 0; i < num_messages; ++i) {
            logger.info("Benchmark message {0} iteration {1} data {2}", i, iteration, i * 2);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double duration_ms = duration.count() / 1000.0;
        iteration_times.push_back(duration_ms);
        
        std::cout << "Iteration " << (iteration + 1) << ": " 
                  << duration_ms << " ms (" 
                  << (num_messages / duration_ms * 1000.0) << " msg/sec)" << std::endl;
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
    
    // Verify buffer content
    auto buffer = logger.buffer();
    std::cout << "Buffer size after test: " << buffer->size() << " messages" << std::endl;
    
    if (buffer->size() > 0) {
        std::cout << "First message: " << buffer->cbegin()->message << std::endl;
        auto last_it = buffer->cend();
        --last_it;
        std::cout << "Last message: " << last_it->message << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "Benchmark completed successfully!" << std::endl;
    std::cout << "Note: This benchmark ran WITH buffer and observer mutex protection." << std::endl;
    std::cout << "This provides thread-safety but may have slight performance overhead." << std::endl;
    std::cout << "Compare results with demo_buffer_benchmark_no_mutex to see the difference." << std::endl;
    
    return 0;
}
