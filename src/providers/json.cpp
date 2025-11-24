#include "serris/providers/json.h"

#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

#include "serris/types/jvalue.h"

using namespace serris::types;

namespace serris::providers {
    namespace {
        // clang-format off
        constexpr const char* control[32] = {
            "\\u0000", "\\u0001", "\\u0002", "\\u0003", "\\u0004", "\\u0005", "\\u0006", "\\u0007",
            "\\b",     "\\t",     "\\n",     "\\u000B", "\\f",     "\\r",     "\\u000E", "\\u000F",
            "\\u0010", "\\u0011", "\\u0012", "\\u0013", "\\u0014", "\\u0015", "\\u0016", "\\u0017",
            "\\u0018", "\\u0019", "\\u001A", "\\u001B", "\\u001C", "\\u001D", "\\u001E", "\\u001F"
        };
        // clang-format on

        static const std::array<std::string, 256> escape_lookup = [] {
            std::array<std::string, 256> table{};

            // control characters
            for (int i = 0; i < 32; ++i) {
                table[i] = control[i];
            }

            // printable ASCII (32..126)
            for (int i = 32; i < 256; ++i) {
                table[i] = std::string(1, static_cast<char>(i));
            }

            // special JSON characters
            table['"'] = "\\\"";
            table['\\'] = "\\\\";
            table['\b'] = "\\b";
            table['\f'] = "\\f";
            table['\n'] = "\\n";
            table['\r'] = "\\r";
            table['\t'] = "\\t";

            return table;
        }();

        inline void skip_whitespace(const std::string& s, size_t& pos) {
            while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) {
                ++pos;
            }
        }

        inline void append_escaped(std::string& out, const std::string& s) {
            out.push_back('"');
            for (unsigned char c : s) {
                out += escape_lookup[c];
            }
            out.push_back('"');
        }

        inline void append_codepoint(std::string& out, unsigned int cp) {
            if (cp <= 0x7F) {
                out += char(cp);
            } else if (cp <= 0x7FF) {
                out += static_cast<char>(0xC0 | (cp >> 6));
                out += static_cast<char>(0x80 | (cp & 0x3F));
            } else if (cp <= 0xFFFF) {
                out += static_cast<char>(0xE0 | (cp >> 12));
                out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (cp & 0x3F));
            } else {
                out += static_cast<char>(0xF0 | (cp >> 18));
                out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (cp & 0x3F));
            }
        }

        void append_number(std::string& out, const jvalue& val) {
            if (val.is_integer()) {
                out += std::to_string(val.try_as_double().value());
                return;
            }

            if (val.is_floating()) {
                double d = val.try_as_double().value();

                std::ostringstream oss;
                oss << std::setprecision(17) << d;

                std::string s = oss.str();
                if (s.find('.') != std::string::npos) {
                    while (!s.empty() && s.back() == '0') {
                        s.pop_back();
                    }

                    if (!s.empty() && s.back() == '.') {
                        s.pop_back();
                    }
                }
                out += s;
            }
        }

        inline std::string parse_string(const std::string& s, size_t& pos) {
            if (pos >= s.size() || s[pos] != '"') {
                throw std::runtime_error("Expected '\"' at start of string");
            }

            ++pos;
            std::string out;

            while (pos < s.size()) {
                char c = s[pos++];

                if (c == '"') {
                    return out;
                }

                if (c == '\\') {
                    if (pos >= s.size()) {
                        throw std::runtime_error("invalid escape in string");
                    }

                    char esc = s[pos++];

                    switch (esc) {
                        case '"':
                            out += '"';
                            break;
                        case '\\':
                            out += '\\';
                            break;
                        case '/':
                            out += '/';
                            break;
                        case 'b':
                            out += '\b';
                            break;
                        case 'f':
                            out += '\f';
                            break;
                        case 'n':
                            out += '\n';
                            break;
                        case 'r':
                            out += '\r';
                            break;
                        case 't':
                            out += '\t';
                            break;
                        case 'u': {
                            if (pos + 3 >= s.size()) {
                                throw std::runtime_error("bad \\u escape");
                            }

                            unsigned cp = 0;
                            for (int i = 0; i < 4; i++) {
                                cp <<= 4;
                                char h = s[pos++];

                                if (h >= '0' && h <= '9') {
                                    cp |= h - '0';
                                } else if (h >= 'a' && h <= 'f') {
                                    cp |= h - 'a' + 10;
                                } else if (h >= 'A' && h <= 'F') {
                                    cp |= h - 'A' + 10;
                                } else {
                                    throw std::runtime_error("bad hex in \\u escape");
                                }
                            }

                            append_codepoint(out, cp);
                            break;
                        }
                        default:
                            out += esc;
                            break;
                    }
                } else {
                    out += c;
                }
            }

            throw std::runtime_error("unterminated string");
        }

        inline jvalue parse_number(const std::string& s, size_t& pos) {
            size_t start = pos;
            bool is_float = false;

            while (pos < s.size() && (std::isdigit(s[pos]) || s[pos] == '.' || s[pos] == 'e' || s[pos] == 'E' || s[pos] == '+' || s[pos] == '-')) {
                if (s[pos] == '.' || s[pos] == 'e' || s[pos] == 'E') {
                    is_float = true;
                }
                ++pos;
            }

            std::string num_str = s.substr(start, pos - start);

            if (is_float) {
                return jvalue{std::stod(num_str)};
            } else {
                return jvalue{std::stoll(num_str)};
            }
        }

        inline void serialize_impl(const jvalue& val, std::string& out, bool pretty, int indent) {
            if (val.is_null()) {
                out += "null";
                return;
            }

            if (std::optional<bool> v = val.try_as_bool()) {
                out += *v ? "true" : "false";
                return;
            }

            if (const std::string* v = val.try_as_string()) {
                append_escaped(out, *v);
                return;
            }

            if (val.is_number()) {
                append_number(out, val);
                return;
            }

            const std::string indent_str(indent, ' ');
            const std::string indent_next(indent + 2, ' ');

            if (const jvalue::jobject* obj = val.try_as_object()) {
                out += '{';
                if (pretty) out += '\n';

                bool first = true;
                for (const auto& [k, v] : *obj) {
                    if (!first) out += pretty ? ",\n" : ",";
                    if (pretty) out += indent_next;

                    append_escaped(out, k);
                    out += pretty ? ": " : ":";
                    serialize_impl(*v.get(), out, pretty, indent + 2);
                    first = false;
                }

                if (pretty) out += '\n' + indent_str;
                out += '}';
                return;
            }

            if (const jvalue::jarray* arr = val.try_as_array()) {
                out += '[';
                if (pretty) out += '\n';

                bool first = true;
                for (const auto& v : *arr) {
                    if (!first) out += pretty ? ",\n" : ",";
                    if (pretty) out += indent_next;

                    serialize_impl(*v.get(), out, pretty, indent + 2);
                    first = false;
                }

                if (pretty) out += '\n' + indent_str;
                out += ']';
                return;
            }

            out += "null";
        }

        inline jvalue deserialize_impl(const std::string& s, size_t& pos) {
            skip_whitespace(s, pos);
            if (pos >= s.size()) {
                throw std::runtime_error("Unexpected end of input");
            }

            char c = s[pos];

            if (s.compare(pos, 4, "null") == 0) {
                pos += 4;
                return jvalue{};
            }

            if (s.compare(pos, 4, "true") == 0) {
                pos += 4;
                return jvalue{true};
            }
            if (s.compare(pos, 5, "false") == 0) {
                pos += 5;
                return jvalue{false};
            }

            if (c == '"') {
                return jvalue{parse_string(s, pos)};
            }

            if (c == '-' || std::isdigit(c)) {
                return parse_number(s, pos);
            }

            if (c == '{') {
                jvalue::jobject obj = jvalue::jobject{};

                ++pos;
                skip_whitespace(s, pos);

                bool first = true;
                while (pos < s.size() && s[pos] != '}') {
                    if (!first) {
                        if (s[pos] == ',') ++pos;
                        skip_whitespace(s, pos);
                    }

                    std::string key = parse_string(s, pos);
                    skip_whitespace(s, pos);

                    if (pos >= s.size() || s[pos] != ':') {
                        throw std::runtime_error("Expected ':' in object");
                    }

                    ++pos;
                    obj[key] = std::make_unique<jvalue>(deserialize_impl(s, pos));

                    first = false;
                    skip_whitespace(s, pos);
                }

                if (pos >= s.size() || s[pos] != '}') {
                    throw std::runtime_error("Expected '}' at end of object");
                }

                ++pos;
                return jvalue{std::move(obj)};
            }

            // array
            if (c == '[') {
                jvalue::jarray arr = jvalue::jarray{};

                ++pos;
                skip_whitespace(s, pos);

                bool first = true;
                while (pos < s.size() && s[pos] != ']') {
                    if (!first) {
                        if (s[pos] == ',') ++pos;
                        skip_whitespace(s, pos);
                    }

                    arr.emplace_back(deserialize_impl(s, pos));
                    first = false;
                    skip_whitespace(s, pos);
                }

                if (pos >= s.size() || s[pos] != ']') {
                    throw std::runtime_error("Expected ']' at end of array");
                }

                ++pos;
                return jvalue{std::move(arr)};
            }

            throw std::runtime_error(std::string("Unexpected character '") + c + "'");
        }
    }

    std::string json::serialize(const jvalue& val, bool pretty) {
        std::string out;
        out.reserve(256);
        serialize_impl(val, out, pretty, 0);
        return out;
    }

    jvalue json::deserialize(const std::string& str) {
        size_t pos = 0;

        jvalue result = deserialize_impl(str, pos);
        skip_whitespace(str, pos);

        if (pos != str.size()) {
            throw std::runtime_error("Extra characters after JSON value");
        }

        return result;
    }

    std::optional<jvalue> json::from_file(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::nullopt;
        }

        try {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return deserialize(buffer.str());
        } catch (...) {
            return std::nullopt;
        }
    }
}