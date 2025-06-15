#pragma once

#include <string>
#include <sstream> // For std::ostringstream
#include <vector>  // For std::vector
#include <map>     // For std::map
#include <utility> // For std::pair
#include <tuple>   // For std::tuple
#include <array>   // For std::array

namespace ustr {

// Forward declarations to enable recursive calls
template <typename T>
std::string to_string(const T& value);

// Default to_string for basic types (relies on std::to_string or ostream operator<<)
template <typename T>
inline std::string to_string(const T& value) {
    // Try to use std::to_string for arithmetic types
    if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(value);
    } else {
        // Fallback to ostream operator<< (only for types that support it)
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
}

// Specialization for std::string
inline std::string to_string(const std::string& value) {
    return value;
}

// Specialization for C-style strings
inline std::string to_string(const char* value) {
    return std::string(value);
}

// Specialization for char (to avoid printing as integer)
inline std::string to_string(char value) {
    return std::string(1, value);
}

// Specialization for std::vector
template <typename T>
inline std::string to_string(const std::vector<T>& vec) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << to_string(vec[i]);
        if (i < vec.size() - 1) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

// Specialization for std::map
template <typename K, typename V>
inline std::string to_string(const std::map<K, V>& m) {
    std::ostringstream oss;
    oss << "{";
    for (auto it = m.begin(); it != m.end(); ++it) {
        oss << to_string(it->first) << ": " << to_string(it->second);
        if (std::next(it) != m.end()) {
            oss << ", ";
        }
    }
    oss << "}";
    return oss.str();
}

// Specialization for std::pair
template <typename T1, typename T2>
inline std::string to_string(const std::pair<T1, T2>& p) {
    std::ostringstream oss;
    oss << "(" << to_string(p.first) << ", " << to_string(p.second) << ")";
    return oss.str();
}

// Specialization for std::tuple
namespace detail {
template <typename Tuple, std::size_t... Is>
std::string tuple_to_string_impl(const Tuple& t, std::index_sequence<Is...>) {
    std::ostringstream oss;
    oss << "(";
    ((oss << (Is == 0 ? "" : ", ") << to_string(std::get<Is>(t))), ...);
    oss << ")";
    return oss.str();
}
} // namespace detail

template <typename... Args>
inline std::string to_string(const std::tuple<Args...>& t) {
    return detail::tuple_to_string_impl(t, std::index_sequence_for<Args...>{});
}

// Specialization for std::array
template <typename T, size_t N>
inline std::string to_string(const std::array<T, N>& arr) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        oss << to_string(arr[i]);
        if (i < arr.size() - 1) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

} // namespace ustr
