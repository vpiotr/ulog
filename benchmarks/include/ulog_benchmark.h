#pragma once

#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <atomic>
#include "ulog/ulog.h"

/**
 * @brief Benchmark infrastructure for ulog performance testing
 */
namespace ulog_benchmark {

/**
 * @brief Statistics collected during benchmark runs
 */
struct BenchmarkStats {
    std::vector<double> iteration_times_ms;
    double min_time_ms = 0.0;
    double max_time_ms = 0.0;
    double avg_time_ms = 0.0;
    double total_time_ms = 0.0;
    double avg_throughput_msg_per_sec = 0.0;
    int num_messages = 0;
    int num_iterations = 0;
    
    void calculate(int messages_per_iteration) {
        if (iteration_times_ms.empty()) return;
        
        num_messages = messages_per_iteration;
        num_iterations = static_cast<int>(iteration_times_ms.size());
        
        min_time_ms = *std::min_element(iteration_times_ms.begin(), iteration_times_ms.end());
        max_time_ms = *std::max_element(iteration_times_ms.begin(), iteration_times_ms.end());
        total_time_ms = std::accumulate(iteration_times_ms.begin(), iteration_times_ms.end(), 0.0);
        avg_time_ms = total_time_ms / num_iterations;
        avg_throughput_msg_per_sec = num_messages / avg_time_ms * 1000.0;
    }
    
    void print_results() const {
        std::cout << std::endl;
        std::cout << "=== RESULTS ===" << std::endl;
        std::cout << "Average time: " << avg_time_ms << " ms" << std::endl;
        std::cout << "Min time: " << min_time_ms << " ms" << std::endl;
        std::cout << "Max time: " << max_time_ms << " ms" << std::endl;
        std::cout << "Average throughput: " << static_cast<int>(avg_throughput_msg_per_sec) << " messages/second" << std::endl;
    }
};

/**
 * @brief Base class for benchmark runners
 */
class BenchmarkRunner {
public:
    explicit BenchmarkRunner(const std::string& name, const std::string& description = "")
        : name_(name), description_(description) {}
    
    virtual ~BenchmarkRunner() = default;
    
    /**
     * @brief Run the benchmark
     * @param num_messages Number of messages per iteration
     * @param num_iterations Number of iterations to run
     * @return Benchmark statistics
     */
    BenchmarkStats run(int num_messages, int num_iterations = 5) {
        print_header();
        setup();
        
        BenchmarkStats stats;
        stats.iteration_times_ms.reserve(num_iterations);
        
        std::cout << "Messages per iteration: " << num_messages << std::endl;
        std::cout << "Running " << num_iterations << " iterations..." << std::endl;
        std::cout << std::endl;
        
        for (int iteration = 0; iteration < num_iterations; ++iteration) {
            iteration_setup(iteration);
            
            auto start = std::chrono::high_resolution_clock::now();
            
            run_iteration(num_messages, iteration);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            double duration_ms = duration.count() / 1000.0;
            stats.iteration_times_ms.push_back(duration_ms);
            
            std::cout << "Iteration " << (iteration + 1) << ": " 
                      << duration_ms << " ms (" 
                      << (num_messages / duration_ms * 1000.0) << " msg/sec)" << std::endl;
                      
            iteration_cleanup(iteration);
        }
        
        stats.calculate(num_messages);
        stats.print_results();
        
        verify_results();
        cleanup();
        print_footer();
        
        return stats;
    }

protected:
    /**
     * @brief Setup called once before all iterations
     */
    virtual void setup() {}
    
    /**
     * @brief Setup called before each iteration
     * @param iteration Current iteration number (0-based)
     */
    virtual void iteration_setup(int iteration) {}
    
    /**
     * @brief Run a single iteration of the benchmark
     * @param num_messages Number of messages to process
     * @param iteration Current iteration number (0-based)
     */
    virtual void run_iteration(int num_messages, int iteration) = 0;
    
    /**
     * @brief Cleanup called after each iteration
     * @param iteration Current iteration number (0-based)
     */
    virtual void iteration_cleanup(int iteration) {}
    
    /**
     * @brief Verify benchmark results (optional)
     */
    virtual void verify_results() {}
    
    /**
     * @brief Cleanup called once after all iterations
     */
    virtual void cleanup() {}
    
    /**
     * @brief Get configuration description for the header
     */
    virtual std::string get_config_description() const = 0;

private:
    void print_header() const {
        std::cout << "=== " << name_ << " ===" << std::endl;
        std::cout << get_config_description() << std::endl;
        if (!description_.empty()) {
            std::cout << description_ << std::endl;
        }
        std::cout << std::endl;
    }
    
    void print_footer() const {
        std::cout << std::endl;
        std::cout << "Benchmark completed successfully!" << std::endl;
    }
    
    std::string name_;
    std::string description_;
};

/**
 * @brief Simple observer that counts messages for benchmarking
 */
class CountingObserver : public ulog::LogObserver {
public:
    void handleNewMessage(const ulog::LogEntry&) override {
        message_count_++;
    }
    
    void handleRegistered(const std::string&) override {}
    void handleUnregistered(const std::string&) override {}
    void handleFlush(const std::string&) override {}
    
    size_t getMessageCount() const { return message_count_; }
    void reset() { message_count_ = 0; }
    
private:
    std::atomic<size_t> message_count_{0};
};

/**
 * @brief Base class for buffer benchmark runners
 */
class BufferBenchmarkRunner : public BenchmarkRunner {
public:
    BufferBenchmarkRunner(const std::string& name, const std::string& description, size_t buffer_capacity = 100000) 
        : BenchmarkRunner(name, description), buffer_capacity_(buffer_capacity) {}

protected:
    void setup() override {
        logger_ = &ulog::getLogger("BenchmarkLogger");
        
        // Disable console output to focus on buffer performance
        logger_->disable_console();
        
        // Enable buffer with specified capacity
        logger_->enable_buffer(buffer_capacity_);
        
        std::cout << "Buffer capacity: " << buffer_capacity_ << std::endl;
    }
    
    void iteration_setup(int) override {
        logger_->clear_buffer();
    }
    
    void verify_results() override {
        auto buffer = logger_->buffer();
        std::cout << "Buffer size after test: " << buffer->size() << " messages" << std::endl;
        
        if (buffer->size() > 0) {
            std::cout << "First message: " << buffer->cbegin()->message << std::endl;
            auto last_it = buffer->cend();
            --last_it;
            std::cout << "Last message: " << last_it->message << std::endl;
        }
    }

protected:
    ulog::Logger* logger_ = nullptr;
    size_t buffer_capacity_;
};

/**
 * @brief Base class for observer benchmark runners
 */
class ObserverBenchmarkRunner : public BenchmarkRunner {
public:
    ObserverBenchmarkRunner(const std::string& name, const std::string& description, int num_observers = 3) 
        : BenchmarkRunner(name, description), num_observers_(num_observers) {}

protected:
    void setup() override {
        logger_ = &ulog::getLogger("ObserverBenchLogger");
        
        // Disable console and buffer to focus purely on observer performance
        logger_->disable_console();
        logger_->disable_buffer();
        
        // Add multiple observers to simulate real-world usage
        observers_.clear();
        observers_.reserve(num_observers_);
        
        for (int i = 0; i < num_observers_; ++i) {
            auto observer = std::make_shared<CountingObserver>();
            observers_.push_back(observer);
            logger_->add_observer(observer);
        }
        
        std::cout << "Number of observers: " << num_observers_ << std::endl;
    }
    
    void iteration_setup(int) override {
        // Reset all observer counts
        for (auto& observer : observers_) {
            observer->reset();
        }
    }
    
    void verify_results() override {
        std::cout << "  Observer counts: ";
        for (size_t i = 0; i < observers_.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << observers_[i]->getMessageCount();
        }
        std::cout << std::endl;
    }
    
    void cleanup() override {
        // Clean up observers
        for (auto& observer : observers_) {
            logger_->remove_observer(observer);
        }
        observers_.clear();
    }

protected:
    ulog::Logger* logger_ = nullptr;
    std::vector<std::shared_ptr<CountingObserver>> observers_;
    int num_observers_;
};

} // namespace ulog_benchmark
