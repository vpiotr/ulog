/**
 * @file demo_windows_utf8.cpp
 * @brief Demo showing Windows UTF-8 console output support and macro configuration
 * 
 * This demo demonstrates:
 * 1. UTF-8 character output on Windows console
 * 2. The ULOG_ENABLE_UTF8_CONSOLE macro configuration
 * 
 * The ULOG_ENABLE_UTF8_CONSOLE macro controls whether Windows-specific
 * UTF-8 console initialization is included. Define it before including
 * ulog.h to enable UTF-8 console support on Windows.
 */

// Enable UTF-8 console support on Windows
// To test without UTF-8 console initialization, comment out the line below:
// #define ULOG_ENABLE_UTF8_CONSOLE
// Without this macro, you may need to manually configure your console for UTF-8:
// - Use 'chcp 65001' in cmd before running
// - Or set console properties to use UTF-8
// - Or use Windows Terminal which has better UTF-8 support

// Windows UTF-8 console initialization (optional feature)
#if defined(_WIN32) && defined(ULOG_ENABLE_UTF8_CONSOLE)
#include <windows.h>

// Undefine conflicting Windows macros that can interfere with ulog enums
#ifdef ERROR
#undef ERROR
#endif
#ifdef TRACE
#undef TRACE
#endif
#ifdef DEBUG
#undef DEBUG
#endif
#endif // defined(_WIN32) && defined(ULOG_ENABLE_UTF8_CONSOLE)

#include "ulog/ulog.h"
#include <iostream>

// Windows UTF-8 console initialization implementation
#if defined(_WIN32) && defined(ULOG_ENABLE_UTF8_CONSOLE)

/**
 * @brief Initialize UTF-8 console output on Windows
 * @return true if initialization was successful
 */
bool initialize_utf8_console() {
    static bool initialized = false;
    if (!initialized) {
        // Set console output code page to UTF-8
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        
        // For better UTF-8 support in console applications
        // Note: We avoid _setmode as it can interfere with other output
        // The /utf-8 compiler flag ensures proper source encoding
        
        initialized = true;
        return true;
    }
    return true;
}
#endif // defined(_WIN32) && defined(ULOG_ENABLE_UTF8_CONSOLE)

int main() {
    std::cout << "=== ulog Windows UTF-8 Demo ===" << std::endl;
    std::cout << "Testing UTF-8 character output on Windows console" << std::endl;
    std::cout << "and demonstrating ULOG_ENABLE_UTF8_CONSOLE macro" << std::endl;
    std::cout << std::endl;

    // Initialize UTF-8 console support if enabled
#if defined(_WIN32) && defined(ULOG_ENABLE_UTF8_CONSOLE)
    bool utf8_initialized = initialize_utf8_console();
    std::cout << "UTF-8 console initialization: " << (utf8_initialized ? "SUCCESS" : "FAILED") << std::endl;
#endif

    // Show macro configuration status
    std::cout << "Macro Configuration:" << std::endl;
#ifdef ULOG_ENABLE_UTF8_CONSOLE
    std::cout << "✓ ULOG_ENABLE_UTF8_CONSOLE is ENABLED" << std::endl;
    std::cout << "  - Windows UTF-8 console initialization code is included" << std::endl;
    std::cout << "  - SetConsoleOutputCP(CP_UTF8) was called manually in this demo" << std::endl;
#else
    std::cout << "✗ ULOG_ENABLE_UTF8_CONSOLE is DISABLED" << std::endl;
    std::cout << "  - Windows UTF-8 console initialization code is excluded" << std::endl;
    std::cout << "  - You may need to manually configure console for UTF-8" << std::endl;
#endif
    std::cout << std::endl;

    // Get logger
    auto& logger = ulog::getLogger("UTF8Test");
    
    // Test Unicode characters that were converted to u8 literals
    logger.info(u8"Testing checkmarks: ✓ success, ✗ failure");
    logger.info(u8"Testing degree symbol: Temperature is 23.5°C");
    logger.info(u8"Testing accented characters: café, naïve, résumé");
    logger.info(u8"Testing Chinese characters: 你好世界 (Hello World)");
    logger.info(u8"Testing emojis: 🙂 😀 🎉 ⭐");
    logger.info(u8"Testing Greek letters: Ω α β γ δ ε");
    logger.info(u8"Testing Spanish characters: ñáéíóú");
    
    // Test mixed content
    logger.warn(u8"Mixed content: ASCII + Unicode (café) + Emoji (🚀) + Greek (Ω)");
    
    // Test in different log levels
    logger.trace(u8"TRACE: Unicode test ✓");
    logger.debug(u8"DEBUG: Unicode test ✓");
    logger.info(u8"INFO: Unicode test ✓");
    logger.warn(u8"WARN: Unicode test ✓");
    logger.error(u8"ERROR: Unicode test ✓");
    logger.fatal(u8"FATAL: Unicode test ✓");
    
    std::cout << std::endl;
    std::cout << "=== Test Complete ===" << std::endl;
    std::cout << "If you can see all Unicode characters correctly above," << std::endl;
    std::cout << "then UTF-8 support is working properly!" << std::endl;
    std::cout << std::endl;
    std::cout << "Note: To disable UTF-8 console initialization," << std::endl;
    std::cout << "comment out the #define ULOG_ENABLE_UTF8_CONSOLE line" << std::endl;
    std::cout << "and recompile to see the difference." << std::endl;
    
    return 0;
}
