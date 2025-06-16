#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <iomanip>
#include "ulog/ulog.h"

int expensive_fibonacci(int n) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Simulate expensive operation
    if (n <= 1) return n;
    return expensive_fibonacci(n-1) + expensive_fibonacci(n-2);
}

double expensive_calculation() {
    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Simulate expensive operation
    double result = 0.0;
    for (int i = 0; i < 1000; ++i) {
        result += std::sin(i) * std::cos(i);
    }
    return result;
}

int main() {
    auto& logger = ulog::getLogger("MessageSupplierDemo");
    
    std::cout << "=== Message Supplier Demo ===" << std::endl;
    std::cout << "Demonstrating zero-cost abstraction with message suppliers" << std::endl << std::endl;
    
    // Test 1: Traditional logging (always evaluates parameters)
    std::cout << "1. Traditional logging with disabled debug level:" << std::endl;
    logger.set_log_level(ulog::LogLevel::WARN); // Disable debug
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 3; ++i) {
        // This will still call expensive_fibonacci even though DEBUG is disabled!
        logger.debug("Traditional: Fibonacci(10) = {}", expensive_fibonacci(10));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto traditional_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Traditional logging time: " << traditional_time.count() << "ms" << std::endl;
    std::cout << "(Notice: expensive calculations were still performed)" << std::endl << std::endl;
    
    // Test 2: Message supplier logging (zero-cost when disabled)
    std::cout << "2. Message supplier logging with disabled debug level:" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 3; ++i) {
        // This will NOT call expensive_fibonacci when DEBUG is disabled!
        logger.debug_supplier([i]() { 
            return "Supplier: Fibonacci(10) = " + std::to_string(expensive_fibonacci(10));
        });
    }
    end = std::chrono::high_resolution_clock::now();
    auto supplier_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Message supplier time: " << supplier_time.count() << "ms" << std::endl;
    std::cout << "(Notice: expensive calculations were NOT performed)" << std::endl << std::endl;
    
    // Test 3: Enable debug and show that suppliers work
    std::cout << "3. Enabling debug level to show suppliers work:" << std::endl;
    logger.set_log_level(ulog::LogLevel::DEBUG);
    
    logger.debug_supplier([]() {
        return "Complex calculation result: " + std::to_string(expensive_calculation());
    });
    
    logger.info_supplier([]() {
        int fib = expensive_fibonacci(8);
        double calc = expensive_calculation();
        return "Combined expensive operations: fib=" + std::to_string(fib) + 
               ", calc=" + std::to_string(calc);
    });
    
    std::cout << std::endl << "=== Performance comparison ===" << std::endl;
    std::cout << "Traditional logging (disabled): " << traditional_time.count() << "ms" << std::endl;
    std::cout << "Message supplier (disabled): " << supplier_time.count() << "ms" << std::endl;
    
    if (supplier_time.count() == 0) {
        if (traditional_time.count() > 0) {
            std::cout << "Speedup: INFINITE! (Zero cost vs " << traditional_time.count() << "ms)" << std::endl;
        } else {
            std::cout << "Both approaches: 0ms (too fast to measure)" << std::endl;
        }
    } else {
        double speedup = (double)traditional_time.count() / supplier_time.count();
        std::cout << "Speedup: " << std::fixed << std::setprecision(1) << speedup << "x faster!" << std::endl;
    }
    
    return 0;
}
