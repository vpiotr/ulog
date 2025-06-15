#include "ulog/ulog.h"
#include "include/ulog_benchmark.h"

// Enable all mutexes for baseline comparison
#define ULOG_USE_MUTEX_FOR_CONSOLE 1
#define ULOG_USE_MUTEX_FOR_BUFFER 1
#define ULOG_USE_MUTEX_FOR_OBSERVERS 1

/**
 * @brief Observer benchmark with mutex protection enabled
 */
class ObserverBenchmarkWithMutex : public ulog_benchmark::ObserverBenchmarkRunner {
public:
    ObserverBenchmarkWithMutex() 
        : ObserverBenchmarkRunner("Observer Performance Benchmark - WITH OBSERVER MUTEX (Single Thread)",
                                 "Testing single-threaded observer notification performance with mutex protection") {}

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
        std::cout << "Note: This benchmark ran WITH observer mutex protection." << std::endl;
        std::cout << "This provides thread-safety but may have slight performance overhead." << std::endl;
        std::cout << "Compare with demo_observer_benchmark_no_mutex to see the performance difference." << std::endl;
    }
    
    std::string get_config_description() const override {
        return "ULOG_USE_MUTEX_FOR_OBSERVERS=1";
    }
};

int main() {
    ObserverBenchmarkWithMutex benchmark;
    benchmark.run(100000, 5);
    return 0;
}
