// Disable observer mutex for performance comparison
#define ULOG_USE_MUTEX_FOR_CONSOLE 1
#define ULOG_USE_MUTEX_FOR_BUFFER 1
#define ULOG_USE_MUTEX_FOR_OBSERVERS 0

#include "ulog/ulog.h"
#include "include/ulog_benchmark.h"

/**
 * @brief Observer benchmark with mutex protection disabled
 */
class ObserverBenchmarkNoMutex : public ulog_benchmark::ObserverBenchmarkRunner {
public:
    ObserverBenchmarkNoMutex() 
        : ObserverBenchmarkRunner("Observer Performance Benchmark - WITHOUT OBSERVER MUTEX (Single Thread)",
                                 "Testing single-threaded observer notification performance without mutex protection") {}

protected:
    void run_iteration(int num_messages, int iteration) override {
        // Write messages that will trigger observer notifications
        for (int i = 0; i < num_messages; ++i) {
            logger_->info("Observer benchmark message {0} iteration {1}", i, iteration);
        }
    }
    
    void iteration_cleanup(int iteration) override {
        ObserverBenchmarkRunner::iteration_cleanup(iteration);
        verify_results();
    }
    
    void cleanup() override {
        ObserverBenchmarkRunner::cleanup();
        std::cout << "Total observer notifications: " << (num_observers_ * 100000) << " per iteration" << std::endl;
        std::cout << std::endl;
        std::cout << "Note: This benchmark ran WITHOUT observer mutex protection." << std::endl;
        std::cout << "In multi-threaded scenarios, this would not be thread-safe for observer operations." << std::endl;
        std::cout << "Compare with demo_observer_benchmark_with_mutex to see the performance difference." << std::endl;
    }
    
    std::string get_config_description() const override {
        return "ULOG_USE_MUTEX_FOR_OBSERVERS=0";
    }
};

int main() {
    ObserverBenchmarkNoMutex benchmark;
    benchmark.run(100000, 5);
    return 0;
}
