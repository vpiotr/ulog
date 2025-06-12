/**
 * @file demo_container_formatting.cpp
 * @brief Step-by-step testing of container formatting in ulog
 */

#include "ulog/ulog.h"
#include <vector>
#include <map>
#include <utility>
#include <string>
#include <sstream>

// Helper functions to format containers manually
// This is the recommended approach for extending ulog with container support

template<typename T>
std::string format_vector(const std::vector<T>& vec) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << vec[i];
    }
    oss << "]";
    return oss.str();
}

template<typename T1, typename T2>
std::string format_pair(const std::pair<T1, T2>& p) {
    std::ostringstream oss;
    oss << "(" << p.first << ", " << p.second << ")";
    return oss.str();
}

template<typename Key, typename Value>
std::string format_map(const std::map<Key, Value>& m) {
    std::ostringstream oss;
    oss << "{";
    auto it = m.begin();
    if (it != m.end()) {
        oss << it->first << ": " << it->second;
        ++it;
        for (; it != m.end(); ++it) {
            oss << ", " << it->first << ": " << it->second;
        }
    }
    oss << "}";
    return oss.str();
}

// Nested container helper - format vector of pairs
template<typename T1, typename T2>
std::string format_vector_of_pairs(const std::vector<std::pair<T1, T2>>& vec) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << format_pair(vec[i]);
    }
    oss << "]";
    return oss.str();
}

// Step 4: Test container formatting using helper functions
int main() {
    auto& logger = ulog::getLogger("ContainerDemo");
    logger.info("Container formatting demo - helper function approach");
    
    // Test simple vector
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    logger.info("Created vector with {?} elements", numbers.size());
    logger.info("Vector contents: {?}", format_vector(numbers));
    
    // Test pair
    std::pair<int, std::string> person{42, "Alice"};
    logger.info("Person: {?}", format_pair(person));
    
    // Test map
    std::map<std::string, int> scores{{"Alice", 95}, {"Bob", 87}};
    logger.info("Scores: {?}", format_map(scores));
    
    // Test nested containers
    std::vector<std::pair<std::string, int>> name_scores{
        {"Alice", 95}, {"Bob", 87}, {"Charlie", 92}
    };
    logger.info("Name-score pairs: {?}", format_vector_of_pairs(name_scores));
    
    // Test individual elements (these work fine)
    logger.info("Individual elements:");
    for (const auto& [name, score] : name_scores) {
        logger.info("  {?}: {?} points", name, score);
    }
    
    return 0;
}