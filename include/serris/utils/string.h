#pragma once

#include <cstdint>
#include <string>

#include "serris/config.h"

namespace serris::utils::string {
    // ASCII whitespace
    constexpr uint32_t TAB = 0x09;
    constexpr uint32_t LF = 0x0A;
    constexpr uint32_t CR = 0x0D;
    constexpr uint32_t SPACE = 0x20;

    // ASCII control characters
    constexpr uint32_t ASCII_CONTROL_START = 0x00;
    constexpr uint32_t ASCII_CONTROL_END = 0x1F;
    constexpr uint32_t DEL = 0x7F;

    constexpr bool is_space(uint32_t cp) {
        return cp == TAB || cp == LF || cp == CR || cp == SPACE;
    }

    constexpr bool is_control(uint32_t cp) {
        return cp < 0x20 || cp == DEL;
    }

    SERRIS_API void sanitize(std::string& str);
    SERRIS_API std::string sanitize(const std::string& str);

    void remove_quotes(std::string& str);
    std::string remove_quotes(const std::string& str);

    void remove_whitespace(std::string& str);
    std::string remove_whitespace(const std::string& str);
}