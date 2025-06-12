#include "utest/utest.h"
#include "ulog/ulog.h"

UTEST_FUNC_DEF2(Formatter, NoParameters) {
    std::string result = ulog::MessageFormatter::format("Simple message");
    UTEST_ASSERT_EQUALS(result, "Simple message");
}

UTEST_FUNC_DEF2(Formatter, AnonymousParameters) {
    std::string result = ulog::MessageFormatter::format("User: {?}, Age: {?}", "John", 25);
    UTEST_ASSERT_EQUALS(result, "User: John, Age: 25");
}

UTEST_FUNC_DEF2(Formatter, PositionalParameters) {
    std::string result = ulog::MessageFormatter::format("Name: {0}, Age: {1}, Name again: {0}", "Alice", 30);
    UTEST_ASSERT_EQUALS(result, "Name: Alice, Age: 30, Name again: Alice");
}

UTEST_FUNC_DEF2(Formatter, MixedParameters) {
    std::string result = ulog::MessageFormatter::format("Anonymous: {?}, Positional: {1}, Another: {?}", "first", "second", "third");
    // Anonymous parameters consume args sequentially first: {?} -> "first", {?} -> "second"
    // Then positional: {1} -> args[1] = "second"
    UTEST_ASSERT_EQUALS(result, "Anonymous: first, Positional: second, Another: second");
}

UTEST_FUNC_DEF2(Formatter, StringConversion) {
    int int_val = 42;
    double double_val = 3.14159;
    std::string str_val = "test";
    const char* cstr_val = "cstring";
    
    std::string result = ulog::MessageFormatter::format("Int: {?}, Double: {?}, String: {?}, CString: {?}", 
                                                         int_val, double_val, str_val, cstr_val);
    
    UTEST_ASSERT_NOT_EQUALS(result.find("Int: 42"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(result.find("Double: 3.14159"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(result.find("String: test"), std::string::npos);
    UTEST_ASSERT_NOT_EQUALS(result.find("CString: cstring"), std::string::npos);
}

UTEST_FUNC_DEF2(Formatter, ExcessParameters) {
    // More parameters than placeholders - should not crash
    std::string result = ulog::MessageFormatter::format("One: {?}", "first", "second", "third");
    UTEST_ASSERT_EQUALS(result, "One: first");
}

UTEST_FUNC_DEF2(Formatter, MissingParameters) {
    // More placeholders than parameters - should leave placeholders unchanged
    std::string result = ulog::MessageFormatter::format("One: {?}, Two: {?}", "first");
    UTEST_ASSERT_EQUALS(result, "One: first, Two: {?}");
}

UTEST_FUNC_DEF2(Formatter, PositionalOutOfRange) {
    // Positional parameter out of range - should leave placeholder unchanged
    std::string result = ulog::MessageFormatter::format("One: {0}, Two: {5}", "first");
    UTEST_ASSERT_EQUALS(result, "One: first, Two: {5}");
}

UTEST_FUNC_DEF2(Formatter, NoPlaceholders) {
    // Parameters provided but no placeholders
    std::string result = ulog::MessageFormatter::format("No placeholders", "unused", "parameters");
    UTEST_ASSERT_EQUALS(result, "No placeholders");
}

UTEST_FUNC_DEF2(Formatter, EmptyMessage) {
    std::string result = ulog::MessageFormatter::format("");
    UTEST_ASSERT_EQUALS(result, "");
}

UTEST_FUNC_DEF2(Formatter, UstrToString) {
    // Test ustr::to_string specializations
    std::string str = "test string";
    const char* cstr = "test cstring";
    int num = 123;
    
    UTEST_ASSERT_EQUALS(ulog::ustr::to_string(str), "test string");
    UTEST_ASSERT_EQUALS(ulog::ustr::to_string(cstr), "test cstring");
    UTEST_ASSERT_EQUALS(ulog::ustr::to_string(num), "123");
}

void test_formatter_register(bool& errorFound) {
    UTEST_FUNC2(Formatter, NoParameters);
    UTEST_FUNC2(Formatter, AnonymousParameters);
    UTEST_FUNC2(Formatter, PositionalParameters);
    UTEST_FUNC2(Formatter, MixedParameters);
    UTEST_FUNC2(Formatter, StringConversion);
    UTEST_FUNC2(Formatter, ExcessParameters);
    UTEST_FUNC2(Formatter, MissingParameters);
    UTEST_FUNC2(Formatter, PositionalOutOfRange);
    UTEST_FUNC2(Formatter, NoPlaceholders);
    UTEST_FUNC2(Formatter, EmptyMessage);
    UTEST_FUNC2(Formatter, UstrToString);
}

int main() {
    UTEST_PROLOG();
    
    test_formatter_register(errorFound);
    
    UTEST_EPILOG();
    
    return 0;
}
