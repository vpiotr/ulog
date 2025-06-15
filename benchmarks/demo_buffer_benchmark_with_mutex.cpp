#include "ulog/ulog.h"
#include "include/ulog_benchmark.h"

// Enable mutex for buffer operations (default)
#define ULOG_USE_MUTEX_FOR_BUFFER 1
// Enable mutex for observer operations (default)
#define ULOG_USE_MUTEX_FOR_OBSERVERS 1
// Keep console mutex enabled (default)

/**
 * @brief Buffer benchmark with mutex protection enabled
 */
class BufferBenchmarkWithMutex : public ulog_benchmark::BufferBenchmarkRunner {
public:
    BufferBenchmarkWithMutex() 
        : BufferBenchmarkRunner("Buffer Write Benchmark - WITH BUFFER/OBSERVER MUTEX (Single Thread)",
                               "Testing single-threaded buffer write performance with mutex protection") {}

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
        std::cout << "Note: This benchmark ran WITH buffer and observer mutex protection." << std::endl;
        std::cout << "This provides thread-safety but may have slight performance overhead." << std::endl;
        std::cout << "Compare results with demo_buffer_benchmark_no_mutex to see the difference." << std::endl;
    }
    
    std::string get_config_description() const override {
        return "ULOG_USE_MUTEX_FOR_BUFFER=1, ULOG_USE_MUTEX_FOR_OBSERVERS=1";
    }
};

int main() {
    BufferBenchmarkWithMutex benchmark;
    benchmark.run(50000, 5);
    return 0;
}
