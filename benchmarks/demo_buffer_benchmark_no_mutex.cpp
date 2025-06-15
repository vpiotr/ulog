#include "ulog/ulog.h"
#include "include/ulog_benchmark.h"

// Disable mutex for buffer operations
#define ULOG_USE_MUTEX_FOR_BUFFER 0
// Disable mutex for observer operations
#define ULOG_USE_MUTEX_FOR_OBSERVERS 0
// Keep console mutex enabled (default)

/**
 * @brief Buffer benchmark with mutex protection disabled
 */
class BufferBenchmarkNoMutex : public ulog_benchmark::BufferBenchmarkRunner {
public:
    BufferBenchmarkNoMutex() 
        : BufferBenchmarkRunner("Buffer Write Benchmark - WITHOUT BUFFER/OBSERVER MUTEX (Single Thread)",
                               "Testing single-threaded buffer write performance without mutex protection") {}

protected:
    void run_iteration(int num_messages, int iteration) override {
        // Write messages to buffer
        for (int i = 0; i < num_messages; ++i) {
            logger_->info("Benchmark message {0} iteration {1} data {2}", i, iteration, i * 2);
        }
    }
    
    void cleanup() override {
        BufferBenchmarkRunner::cleanup();
        std::cout << std::endl;
        std::cout << "Note: This benchmark ran WITHOUT buffer and observer mutex protection." << std::endl;
        std::cout << "In multi-threaded scenarios, this would not be thread-safe." << std::endl;
        std::cout << "Compare results with demo_buffer_benchmark_with_mutex to see the difference." << std::endl;
    }
    
    std::string get_config_description() const override {
        return "ULOG_USE_MUTEX_FOR_BUFFER=0, ULOG_USE_MUTEX_FOR_OBSERVERS=0";
    }
};

int main() {
    BufferBenchmarkNoMutex benchmark;
    benchmark.run(50000, 5);
    return 0;
}
