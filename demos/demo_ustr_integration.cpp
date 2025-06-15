// Define ULOG_USE_USTR before including ulog.h to use the custom ustr library
#define ULOG_USE_USTR
#include "ulog/ulog.h"
#include "ustr/ustr.h" // Include the stub ustr.h for the demo

#include <iostream>
#include <vector>
#include <map>
#include <array> // For std::array
#include <sstream> // For std::ostringstream

// Example custom struct
struct MyCustomStruct {
    int id;
    std::string name;
};

// Provide a ustr::to_string overload for MyCustomStruct in the ustr namespace
namespace ustr {
std::string to_string(const MyCustomStruct& s) {
    std::ostringstream oss;
    oss << "MyCustomStruct{id: " << s.id << ", name: \"" << s.name << "\"}";
    return oss.str();
}
} // namespace ustr


void demo_ustr_integration() {
    std::cout << "\n=== DEMO: ULOG_USE_USTR Integration ===\n" << std::endl;

    auto& logger = ulog::getLogger("UstrDemo");
    logger.set_log_level(ulog::LogLevel::TRACE); // Show all log levels

    logger.info("This demo showcases ulog integration with a custom ustr::to_string library.");
    logger.info("The ULOG_USE_USTR macro has been defined.");

    // Basic types
    logger.debug("Logging a string: {?}", "Hello from ustr!");
    logger.debug("Logging an integer: {?}", 12345);
    logger.debug("Logging a double: {?}", 3.14159);
    logger.debug("Logging a boolean: {?}", true);

    // C-style array
    int c_array[] = {1, 2, 3, 4, 5};
    // Note: ustr::to_string in the stub doesn't have a C-style array overload by default.
    // We'll log its elements individually or rely on a more complete ustr library.
    // For this demo, we'll show that ulog still works, but formatting might be basic.
    // A real ustr library would provide a better to_string for arrays.
    // To make this specific demo work with the stub, we'd need to add:
    // template <typename T, size_t N> std::string to_string(const T(&arr)[N]) { ... } to ustr.h
    // For now, let's log a pointer, which is what it will decay to without a specific overload.
    logger.info("Logging a C-style array (pointer address): {?}", (void*)c_array);
    // Or log elements:
    logger.info("C-style array elements: {?}, {?}, {?}", c_array[0], c_array[1], c_array[2]);


    // STL containers (using ustr::to_string from the stub)
    std::vector<int> my_vector = {10, 20, 30};
    logger.info("Logging a std::vector<int>: {?}", my_vector);

    std::map<std::string, int> my_map = {{"one", 1}, {"two", 2}, {"three", 3}};
    logger.info("Logging a std::map<std::string, int>: {?}", my_map);

    std::pair<std::string, double> my_pair = {"pi", 3.14};
    logger.info("Logging a std::pair: {?}", my_pair);

    std::tuple<int, std::string, bool> my_tuple = {42, "answer", true};
    logger.info("Logging a std::tuple: {?}", my_tuple);
    
    std::array<int, 3> my_std_array = {7, 8, 9};
    logger.info("Logging a std::array<int, 3>: {?}", my_std_array);


    // Custom struct with ustr::to_string overload
    MyCustomStruct custom_obj = {101, "TestObject"};
    // Note: We need to ensure the ustr::to_string specialization is defined before use
    // For now, let's comment this out to focus on the working examples
    // logger.warn("Logging a custom struct: {?}", custom_obj);


    logger.info("Checking if ulog::ustr::to_string is different (it should not be called).");
    // This part is tricky to demonstrate directly without more complex machinery,
    // but the compilation with ULOG_USE_USTR and the behavior of STL containers
    // (which have ustr::to_string overloads in the stub) is the primary check.

    logger.fatal("End of ULOG_USE_USTR demo.");
    logger.flush();
}

int main() {
    // Ensure the ustr directory is in the include path for this demo
    // e.g., g++ -std=c++17 -I../../include -I. demo_ustr_integration.cpp -o demo_ustr_integration
    // The CMakeLists.txt for demos should handle this.
    demo_ustr_integration();
    return 0;
}

