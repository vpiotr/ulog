/**
 * @file demo_custom_formatting.cpp
 * @brief Demonstration of custom formatting support in ulog
 * 
 * This demo shows how to provide custom formatting functionality for both
 * primitive and user-defined types by implementing operator<< or specializing ustr::to_string.
 */

#include "ulog/ulog.h"
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <iomanip>

/**
 * @brief Custom wrapper for integers with hexadecimal formatting
 */
class HexInt {
public:
    explicit HexInt(int value) : value_(value) {}
    
    friend std::ostream& operator<<(std::ostream& os, const HexInt& hex) {
        os << "0x" << std::hex << std::uppercase << hex.value_ << std::dec;
        return os;
    }
    
    int getValue() const { return value_; }
    
private:
    int value_;
};

/**
 * @brief Custom wrapper for booleans with Yes/No formatting
 */
class YesNoBoolean {
public:
    explicit YesNoBoolean(bool value) : value_(value) {}
    
    friend std::ostream& operator<<(std::ostream& os, const YesNoBoolean& yn) {
        os << (yn.value_ ? "Yes" : "No");
        return os;
    }
    
    bool getValue() const { return value_; }
    
private:
    bool value_;
};

/**
 * @brief Custom wrapper for floats with precision formatting
 */
class PrecisionFloat {
public:
    PrecisionFloat(float value, int precision = 2) : value_(value), precision_(precision) {}
    
    friend std::ostream& operator<<(std::ostream& os, const PrecisionFloat& pf) {
        os << std::fixed << std::setprecision(pf.precision_) << pf.value_;
        return os;
    }
    
    float getValue() const { return value_; }
    int getPrecision() const { return precision_; }
    
private:
    float value_;
    int precision_;
};

/**
 * @brief Custom wrapper for percentages
 */
class Percentage {
public:
    explicit Percentage(double value) : value_(value) {}
    
    friend std::ostream& operator<<(std::ostream& os, const Percentage& pct) {
        os << std::fixed << std::setprecision(1) << (pct.value_ * 100.0) << "%";
        return os;
    }
    
    double getValue() const { return value_; }
    
private:
    double value_;
};

// Custom specializations for primitive types using ustr::to_string
namespace ulog {
namespace ustr {
    // Specialization for binary representation of integers
    template<>
    inline std::string to_string<std::pair<int, const char*>>(const std::pair<int, const char*>& value) {
        if (std::string(value.second) == "binary") {
            std::ostringstream oss;
            oss << "0b";
            for (int i = 31; i >= 0; --i) {
                oss << ((value.first >> i) & 1);
            }
            return oss.str();
        }
        return std::to_string(value.first);
    }
}
}

/**
 * @brief Basic custom class with operator<< overload
 */
class Person {
public:
    Person(const std::string& name, int age) : name_(name), age_(age) {}
    
    // Provide operator<< for ulog support
    friend std::ostream& operator<<(std::ostream& os, const Person& person) {
        os << "Person(name=" << person.name_ << ", age=" << person.age_ << ")";
        return os;
    }
    
    const std::string& getName() const { return name_; }
    int getAge() const { return age_; }
    
private:
    std::string name_;
    int age_;
};

/**
 * @brief Advanced custom class with JSON-like output
 */
class UserAccount {
public:
    UserAccount(int id, const std::string& email, const std::vector<std::string>& roles)
        : id_(id), email_(email), roles_(roles) {}
    
    friend std::ostream& operator<<(std::ostream& os, const UserAccount& account) {
        os << "{\"id\":" << account.id_ 
           << ",\"email\":\"" << account.email_ << "\""
           << ",\"roles\":[";
        
        for (size_t i = 0; i < account.roles_.size(); ++i) {
            if (i > 0) os << ",";
            os << "\"" << account.roles_[i] << "\"";
        }
        os << "]}";
        return os;
    }
    
private:
    int id_;
    std::string email_;
    std::vector<std::string> roles_;
};

/**
 * @brief Custom Point class with specialized coordinate formatting
 */
class Point {
public:
    Point(double x, double y) : x_(x), y_(y) {}
    
    friend std::ostream& operator<<(std::ostream& os, const Point& point) {
        os << "(" << point.x_ << ", " << point.y_ << ")";
        return os;
    }
    
    double getX() const { return x_; }
    double getY() const { return y_; }
    
private:
    double x_, y_;
};

/**
 * @brief Custom class using ustr::to_string specialization instead of operator<<
 */
class SpecialType {
public:
    SpecialType(const std::string& data) : data_(data) {}
    
    const std::string& getData() const { return data_; }
    
private:
    std::string data_;
};

// Specialize ustr::to_string for SpecialType
namespace ulog {
namespace ustr {
    template<>
    inline std::string to_string<SpecialType>(const SpecialType& value) {
        return "SpecialType{data=\"" + value.getData() + "\"}";
    }
}
}

/**
 * @brief Helper function to format standard containers manually
 */
template<typename Container>
std::string format_container(const Container& container, const std::string& name) {
    std::ostringstream oss;
    oss << name << "[";
    auto it = container.begin();
    if (it != container.end()) {
        oss << *it;
        ++it;
        for (; it != container.end(); ++it) {
            oss << ", " << *it;
        }
    }
    oss << "]";
    return oss.str();
}

/**
 * @brief Demo function showcasing primitive type custom formatting
 */
void demo_primitive_formatting() {
    std::cout << "\n=== DEMO: Primitive Type Custom Formatting ===" << std::endl;
    
    auto& logger = ulog::getLogger("PrimitiveDemo");
    logger.set_log_level(ulog::LogLevel::DEBUG);
    
    // Regular primitive types
    int number = 255;
    bool flag = true;
    float pi = 3.14159f;
    
    logger.info("Standard formatting - Number: {?}, Flag: {?}, Pi: {?}", number, flag, pi);
    
    // Custom formatted primitives using wrapper classes
    HexInt hexNum(255);
    YesNoBoolean ynFlag(true);
    YesNoBoolean ynFlagFalse(false);
    PrecisionFloat precisionPi(3.14159f, 4);
    PrecisionFloat currency(123.456f, 2);
    Percentage completion(0.85);
    
    logger.info("Custom formatting - Hex: {?}, Yes/No: {?}/{?}", hexNum, ynFlag, ynFlagFalse);
    logger.info("Precision float - Pi: {?}, Currency: ${?}", precisionPi, currency);
    logger.info("Percentage formatting - Completion: {?}", completion);
    
    // Using template specialization for binary representation
    std::pair<int, const char*> binaryNum(42, "binary");
    logger.debug("Binary representation of 42: {?}", binaryNum);
    
    // Demonstrating multiple formatting options for the same value
    int value = 1024;
    HexInt hexValue(value);
    std::pair<int, const char*> binaryValue(value, "binary");
    
    logger.info("Value {?} in different formats:", value);
    logger.info("  Decimal: {?}", value);
    logger.info("  Hexadecimal: {?}", hexValue);
    logger.info("  Binary: {?}", binaryValue);
    
    // Practical examples
    int errorCode = 404;
    bool isConnected = false;
    float temperature = 23.7f;
    double successRate = 0.987;
    
    HexInt hexErrorCode(errorCode);
    YesNoBoolean connectionStatus(isConnected);
    PrecisionFloat tempDisplay(temperature, 1);
    Percentage successDisplay(successRate);
    
    logger.warn("HTTP Error: {?} (hex: {?})", errorCode, hexErrorCode);
    logger.info("Connection status: {?}", connectionStatus);
    logger.info("Temperature: {?}°C", tempDisplay);
    logger.info("Success rate: {?}", successDisplay);
}

/**
 * @brief Demo function showcasing basic custom class support
 */
void demo_basic_custom_class() {
    std::cout << "\n=== DEMO: Basic Custom Class Support ===" << std::endl;
    
    auto& logger = ulog::getLogger("CustomDemo");
    
    Person alice("Alice", 30);
    Person bob("Bob", 25);
    
    logger.info("Created user: {?}", alice);
    logger.info("User details - Name: {0}, Age: {1}", alice.getName(), alice.getAge());
    logger.info("Comparing users: {?} vs {?}", alice, bob);
    
    // Test with anonymous and positional parameters
    logger.warn("User {0} is older than {1}", alice, bob);
}

/**
 * @brief Demo function showcasing advanced custom class with JSON output
 */
void demo_advanced_custom_class() {
    std::cout << "\n=== DEMO: Advanced Custom Class (JSON-like) ===" << std::endl;
    
    auto& logger = ulog::getLogger("AccountDemo");
    
    UserAccount admin(1, "admin@example.com", {"admin", "user"});
    UserAccount guest(2, "guest@example.com", {"guest"});
    
    logger.info("Admin account: {?}", admin);
    logger.info("Guest account: {?}", guest);
    logger.error("Access denied for account: {?}", guest);
}

/**
 * @brief Demo function showcasing geometric types
 */
void demo_geometric_types() {
    std::cout << "\n=== DEMO: Geometric Types ===" << std::endl;
    
    auto& logger = ulog::getLogger("GeometryDemo");
    logger.set_log_level(ulog::LogLevel::DEBUG);  // Enable DEBUG messages for this demo
    
    Point origin(0.0, 0.0);
    Point destination(10.5, 7.3);
    
    logger.info("Starting at point: {?}", origin);
    logger.info("Moving to point: {?}", destination);
    logger.debug("Distance calculation between {?} and {?}", origin, destination);
    
    // Calculate and log distance
    double dx = destination.getX() - origin.getX();
    double dy = destination.getY() - origin.getY();
    double distance = std::sqrt(dx*dx + dy*dy);
    
    logger.info("Distance from {?} to {?} is {?}", origin, destination, distance);
}

/**
 * @brief Demo function showcasing ustr::to_string specialization
 */
void demo_specialized_to_string() {
    std::cout << "\n=== DEMO: ustr::to_string Specialization ===" << std::endl;
    
    auto& logger = ulog::getLogger("SpecialDemo");
    logger.set_log_level(ulog::LogLevel::DEBUG);  // Enable DEBUG messages for this demo
    
    SpecialType obj1("important_data");
    SpecialType obj2("configuration");
    
    logger.info("Processing object: {?}", obj1);
    logger.warn("Invalid configuration in object: {?}", obj2);
    logger.debug("Objects: {?}, {?}", obj1, obj2);
}

/**
 * @brief Demo function showcasing container handling approaches
 */
void demo_container_support() {
    std::cout << "\n=== DEMO: Container Support ===" << std::endl;
    
    auto& logger = ulog::getLogger("ContainerDemo");
    logger.set_log_level(ulog::LogLevel::DEBUG);  // Enable DEBUG messages for this demo
    
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}, {"Charlie", 92}};
    
    // Standard containers don't have operator<< by default, so we use helpers
    logger.info("Processing {?} items", numbers.size());
    logger.info("Numbers: {?}", format_container(numbers, "vector"));
    
    // For maps, we need a custom formatter
    std::ostringstream map_str;
    map_str << "map{";
    auto it = scores.begin();
    if (it != scores.end()) {
        map_str << it->first << ":" << it->second;
        ++it;
        for (; it != scores.end(); ++it) {
            map_str << ", " << it->first << ":" << it->second;
        }
    }
    map_str << "}";
    
    logger.info("Scores: {?}", map_str.str());
    
    // Individual elements work fine
    for (const auto& [name, score] : scores) {
        logger.debug("Student {?} scored {?} points", name, score);
    }
}

/**
 * @brief Demo function showcasing error handling for non-streamable types
 */
void demo_error_handling() {
    std::cout << "\n=== DEMO: Error Handling ===" << std::endl;
    
    auto& logger = ulog::getLogger("ErrorDemo");
    
    // Custom class with private constructor and no operator<<
    class NonStreamable {
    private:
        int value_;
    public:
        NonStreamable(int value) : value_(value) {}
        int getValue() const { return value_; }
        // No operator<< defined - this would cause compilation error if used directly
    };
    
    // Instead of logging non-streamable objects directly, extract relevant data
    NonStreamable obj(42);
    logger.info("Object value: {?}", obj.getValue());
    logger.warn("Processing object with value: {?}", obj.getValue());
    
    // Show best practices for complex objects
    logger.info("Object state: value={?}, valid={?}", obj.getValue(), true);
}

/**
 * @brief Demo function showcasing performance considerations
 */
void demo_performance_tips() {
    std::cout << "\n=== DEMO: Performance Tips ===" << std::endl;
    
    auto& logger = ulog::getLogger("PerfDemo");
    
    // Efficient: simple types
    int counter = 0;
    logger.trace("Counter: {?}", counter);
    
    // Efficient: string conversion done only when needed
    Person person("Performance", 100);
    logger.debug("Processing person: {?}", person);  // operator<< called only if DEBUG level is enabled
    
    // Less efficient: complex string building when not needed
    logger.set_log_level(ulog::LogLevel::INFO);  // This will filter out TRACE messages
    
    // This trace message won't appear, but the expensive operation would still happen:
    // logger.trace("Expensive operation result: {?}", expensive_computation());
    
    // Better approach: check log level first for expensive operations
    if (logger.get_log_level() <= ulog::LogLevel::TRACE) {
        // Only do expensive work if trace logging is enabled
        std::string expensive_result = "expensive_computation_result";
        logger.trace("Expensive operation result: {?}", expensive_result);
    }
    
    logger.info("Performance demo completed");
}

/**
 * @brief Main demo function
 */
int main() {
    std::cout << "=== ulog Custom Formatting Demo ===" << std::endl;
    std::cout << "This demo showcases how to provide custom formatting functionality for both primitive and user-defined types." << std::endl;
    
    try {
        demo_primitive_formatting();
        demo_basic_custom_class();
        demo_advanced_custom_class();
        demo_geometric_types();
        demo_specialized_to_string();
        demo_container_support();
        demo_error_handling();
        demo_performance_tips();
        
        std::cout << "\n=== Demo completed successfully! ===" << std::endl;
        std::cout << "\nKey Takeaways:" << std::endl;
        std::cout << "1. Use wrapper classes for custom primitive formatting (hex, percentage, etc.)" << std::endl;
        std::cout << "2. Implement operator<< for your classes to enable ulog support" << std::endl;
        std::cout << "3. Keep operator<< output concise and readable" << std::endl;
        std::cout << "4. For special cases, specialize ulog::ustr::to_string<YourType>" << std::endl;
        std::cout << "5. Standard containers need manual formatting helpers" << std::endl;
        std::cout << "6. Consider performance when logging complex objects" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
