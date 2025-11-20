#include "serris/utils/string.h"

namespace serris::utils::string {
    void sanitize(std::string& str) {
        std::string result;
        result.reserve(str.size());

        for (char c : str) {
            if ((c < 0x20 && c != SPACE) || c == DEL) {
                continue;
            }

            if (c == '\t' || c == '\n' || c == '\r') {
                c = ' ';
            }

            result += c;
        }

        str.swap(result);
    }

    std::string sanitize(const std::string& str) {
        std::string result = str;
        sanitize(result);
        return result;
    }

    void remove_quotes(std::string& str) {
        str.erase(std::remove(str.begin(), str.end(), '"'), str.end());
    }

    std::string remove_quotes(const std::string& str) {
        std::string result = str;
        remove_quotes(result);
        return result;
    }

    void remove_whitespace(std::string& str) {
        size_t dst = 0;
        bool need_space = false;

        for (size_t src = 0; src < str.size(); ++src) {
            char c = str[src];

            if ((c < 0x20 || c == 0x7F) && c != SPACE && c != '\t' && c != '\n' && c != '\r') {
                continue;
            }

            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                need_space = true;
                continue;
            }

            if (need_space && dst > 0) {
                str[dst++] = ' ';
            }

            str[dst++] = c;
            need_space = false;
        }

        // remove trailing space
        if (dst > 0 && str[dst - 1] == ' ') {
            --dst;
        }

        str.resize(dst);
    }

    std::string remove_whitespace(const std::string& str) {
        std::string result = str;
        remove_whitespace(result);
        return result;
    }
}