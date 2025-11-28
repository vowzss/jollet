#pragma once

#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <tsl/robin_map.h>

#include "JType.h"

namespace serris {
    struct JValue {
        struct JObjectEntries;
        struct JArrayElements;

      private:
        using JArray = std::vector<JType>;
        using JObject = tsl::robin_map<std::string, JType>;

        template <typename T>
        using enable_if_integer_t =
            std::enable_if_t<std::is_integral<T>::value && !std::is_same_v<T, bool> && !std::is_convertible_v<T, const char*>, int>;

        template <typename T>
        using enable_if_floating_t = std::enable_if_t<std::is_floating_point<T>::value, int>;

      private:
        std::variant<std::monostate, bool, std::string, int64_t, double, JObject, JArray> value;

      public:
        // --- constructors ---
        inline explicit JValue() noexcept : value(std::monostate{}) {}
        JValue(const JValue& other);
        JValue(JValue&& other) noexcept;

        inline explicit JValue(bool value) noexcept : value(value) {}

        inline explicit JValue(const std::string& value) : value(value) {}
        inline explicit JValue(std::string&& value) noexcept : value(std::move(value)) {}
        inline explicit JValue(const char* value) : value(std::string(value)) {}

        template <typename T, enable_if_integer_t<T> = 0>
        inline explicit JValue(T value) noexcept : value(static_cast<int64_t>(value)) {}

        template <typename T, enable_if_floating_t<T> = 0>
        inline explicit JValue(T value) noexcept : value(static_cast<double>(value)) {}

        // --- factories ---
        static JValue makeArray() noexcept {
            JValue val;
            val.value = JArray{};
            return val;
        }

        static JValue makeObject() noexcept {
            JValue val;
            val.value = JObject{};
            return val;
        }

        // --- helpers (common types) ---
        std::optional<bool> tryAsBool() const noexcept;
        bool asBool() const;

        const std::string* tryAsString() const noexcept;
        const std::string& asString() const;

        std::optional<int> tryAsInt() const noexcept;
        int asInt() const;

        std::optional<short> tryAsShort() const noexcept;
        short asShort() const;

        std::optional<long> tryAsLong() const noexcept;
        long asLong() const;

        std::optional<float> tryAsFloat() const noexcept;
        float asFloat() const;

        std::optional<double> tryAsDouble() const noexcept;
        double asDouble() const;

        // --- helpers (array) ---
        const JValue* find(size_t idx) const noexcept;

        JValue& emplace(JValue&& val);
        JArrayElements elements() const;

        // --- helpers (object) ---
        const JValue* find(const std::string& key) const noexcept;

        JObjectEntries entries() const;

        // --- utilities ---
        std::unique_ptr<JValue> clone() const;

        constexpr bool isNull() const noexcept;
        constexpr bool isBool() const noexcept;
        constexpr bool isString() const noexcept;

        constexpr bool isInteger() const noexcept;
        constexpr bool isFloating() const noexcept;
        constexpr bool isNumber() const noexcept;

        constexpr bool isArray() const noexcept;
        constexpr bool isObject() const noexcept;

        // --- operators (deep-copy + move) ---
        JValue& operator=(const JValue& other);
        JValue& operator=(JValue&& other) noexcept;

        // --- operators (equality) ---
        bool operator==(const JValue& other) const noexcept;
        bool operator!=(const JValue& other) const noexcept;

        // --- operators (accessors) ---
        JValue& operator[](size_t idx);
        const JValue& operator[](size_t idx) const;

        JValue& operator[](const std::string& key);
        const JValue& operator[](const std::string& key) const;

        inline void print() const noexcept {
            std::visit(
                [](auto&& val) {
                    using T = std::decay_t<decltype(val)>;

                    if constexpr (std::is_same_v<T, std::monostate>) {
                        std::cout << "Type: null\n";
                    } else if constexpr (std::is_same_v<T, bool>) {
                        std::cout << "Type: bool, Value: " << std::boolalpha << val << "\n";
                    } else if constexpr (std::is_same_v<T, int64_t>) {
                        std::cout << "Type: int64_t, Value: " << val << "\n";
                    } else if constexpr (std::is_same_v<T, double>) {
                        std::cout << "Type: double, Value: " << val << "\n";
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        std::cout << "Type: string, Value: \"" << val << "\"\n";
                    } else if constexpr (std::is_same_v<T, JObject>) {
                        std::cout << "Type: JObject, Size: " << val.size() << "\n";
                    } else if constexpr (std::is_same_v<T, JArray>) {
                        std::cout << "Type: JArray, Size: " << val.size() << "\n";
                    } else {
                        std::cout << "Unknown type\n";
                    }
                },
                value);
        };
    };

    // --- ---
    inline void swap(JType& a, JType& b) noexcept {
        a.swap(b);
    }

    // --- iterators ---
    struct JValue::JArrayElements {
        struct JArrayElement {
            size_t idx;
            const JValue& value;
        };

      private:
        const JArray& arr;

      public:
        explicit JArrayElements(const JArray& val) : arr(val) {}

        struct Iterator;
        Iterator begin() const;
        Iterator end() const;
    };

    struct JValue::JObjectEntries {
        struct JObjectEntry {
            const std::string& key;
            const JValue& value;
        };

      private:
        const JObject& obj;

      public:
        explicit JObjectEntries(const JObject& val) : obj(val) {}

        struct Iterator;
        Iterator begin() const;
        Iterator end() const;
    };
}

#include "JValue.inl"
