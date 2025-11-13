#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

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
constexpr bool is_control(uint32_t cp) { return cp < 0x20 || cp == DEL; }

inline void sanitize(std::string &str) {
  std::string result;
  result.reserve(str.size());

  for (char c : str) {
    if ((c < 0x20 && c != SPACE) || c == DEL)
      continue;

    if (c == '\t' || c == '\n' || c == '\r')
      c = ' ';

    result += c;
  }

  str.swap(result);
}

inline std::string sanitize(const std::string &str) {
  std::string result = str;
  sanitize(result);
  return result;
}

inline void remove_quotes(std::string &str) {
  str.erase(std::remove(str.begin(), str.end(), '"'), str.end());
}

inline std::string remove_quotes(const std::string &str) {
  std::string result = str;
  remove_quotes(result);
  return result;
}

inline void remove_whitespace(std::string &str) {
  size_t dst = 0;
  bool need_space = false;

  for (size_t src = 0; src < str.size(); ++src) {
    char c = str[src];

    if ((c < 0x20 || c == 0x7F) && c != SPACE && c != '\t' && c != '\n' &&
        c != '\r')
      continue;

    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      need_space = true;
      continue;
    }

    if (need_space && dst > 0)
      str[dst++] = ' ';

    str[dst++] = c;
    need_space = false;
  }

  // remove trailing space
  if (dst > 0 && str[dst - 1] == ' ') {
    --dst;
  }

  str.resize(dst);
}

inline std::string remove_whitespace(const std::string &str) {
  std::string result = str;
  remove_whitespace(result);
  return result;
}
} // namespace serris::utils::string