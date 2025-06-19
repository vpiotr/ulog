/**
 * @file demo_windows_utf8.cpp
 * @brief Demo showing Windows UTF-8 console output support
 * 
 * This demo demonstrates that ulog properly handles UTF-8 characters 
 * on Windows console output, including Unicode characters that were 
 * converted to u8 string literals.
 */

#include "ulog/ulog.h"
#include <iostream>

int main() {
    std::cout << "=== ulog Windows UTF-8 Demo ===" << std::endl;
    std::cout << "Testing UTF-8 character output on Windows console" << std::endl;
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
    std::cout << "then UTF-8 support is working properly on Windows!" << std::endl;
    
    return 0;
}
