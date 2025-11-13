#pragma once

#include <array>
#include <string>

#include "serris/types/jvalue.h"

namespace serris::providers {
class json {
private:
  static const std::array<std::string_view, 256> lookup_;

public:
  static std::string stringify(const types::jvalue &val, bool pretty = false) {
    return pretty ? stringify_pretty(val, 0) : stringify_compact(val);
  }

private:
  static std::string escape(const std::string &s) {
    std::string result;
    result.reserve(s.size() + 2);
    result.push_back('"');

    for (unsigned char c : s)
      result += lookup_[c];

    result.push_back('"');
    return result;
  }

  static std::string stringify_compact(const types::jvalue &val) {
    if (val.is_null())
      return "null";

    if (auto v = val.try_as_bool())
      return *v ? "true" : "false";

    if (auto v = val.try_as<int64_t>())
      return std::to_string(*v);

    if (auto v = val.try_as_double())
      return std::to_string(*v);

    if (auto v = val.try_as_string())
      return escape(*v);

    if (auto v = val.try_as_object()) {
      std::string result;
      result.reserve(v->size() * 16 + 2);
      result += '{';

      bool first = true;
      for (const auto &[k, v] : *v) {
        if (!first)
          result += ',';

        result += escape(k);
        result += ':';
        result += stringify_compact(v);
        first = false;
      }

      result += '}';
      return result;
    }

    if (auto v = val.try_as_array()) {
      std::string result;
      result.reserve(v->size() * 4 + 2);
      result += '[';
      bool first = true;

      for (const auto &item : *v) {
        if (!first)
          result += ',';

        result += stringify_compact(item);
        first = false;
      }

      result += ']';
      return result;
    }

    return "null";
  }

  static std::string stringify_pretty(const types::jvalue &val, int indent) {
    if (val.is_null())
      return "null";

    if (auto v = val.try_as_bool())
      return *v ? "true" : "false";

    if (auto v = val.try_as<int64_t>())
      return std::to_string(*v);

    if (auto v = val.try_as<double>())
      return std::to_string(*v);

    if (auto v = val.try_as<std::string>())
      return escape(*v);

    const std::string indent_str(indent, ' ');
    const std::string indent_next(indent + 2, ' ');

    if (auto v = val.try_as_object()) {
      std::string result;
      result.reserve(v->size() * 16 + indent + 4);
      result += "{\n";

      bool first = true;
      for (const auto &[k, v] : *v) {
        if (!first)
          result += ",\n";

        result += indent_next;
        result += escape(k);
        result += ": ";
        result += stringify_pretty(v, indent + 2);
        first = false;
      }

      result += "\n";
      result += indent_str;
      result += "}";
      return result;
    }

    if (auto v = val.try_as_array()) {
      std::string result;
      result.reserve(v->size() * 4 + indent + 4);
      result += "[\n";

      bool first = true;
      for (const auto &item : *v) {
        if (!first)
          result += ",\n";

        result += indent_next;
        result += stringify_pretty(item, indent + 2);
        first = false;
      }

      result += "\n";
      result += indent_str;
      result += "]";
      return result;
    }

    return "null";
  }
};

const std::array<std::string_view, 256> json::lookup_ = [] {
  std::array<std::string_view, 256> table{};

  // Control chars
  static constexpr const char *control[32] = {
      "\\u0000", "\\u0001", "\\u0002", "\\u0003", "\\u0004", "\\u0005",
      "\\u0006", "\\u0007", "\\b",     "\\t",     "\\n",     "\\u000B",
      "\\f",     "\\r",     "\\u000E", "\\u000F", "\\u0010", "\\u0011",
      "\\u0012", "\\u0013", "\\u0014", "\\u0015", "\\u0016", "\\u0017",
      "\\u0018", "\\u0019", "\\u001A", "\\u001B", "\\u001C", "\\u001D",
      "\\u001E", "\\u001F"};
  for (int i = 0; i < 32; ++i)
    table[i] = control[i];

  // ASCII chars
  static char ascii[256 - 32][2];
  for (int i = 32; i < 256; ++i) {
    ascii[i - 32][0] = static_cast<char>(i);
    ascii[i - 32][1] = '\0';
    table[i] = ascii[i - 32];
  }

  // Special JSON chars
  table['\"'] = "\\\"";
  table['\\'] = "\\\\";
  table['\b'] = "\\b";
  table['\f'] = "\\f";
  table['\n'] = "\\n";
  table['\r'] = "\\r";
  table['\t'] = "\\t";

  return table;
}();
} // namespace serris::providers