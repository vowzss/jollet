#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <tsl/robin_map.h>

#include "jtype.h"

namespace serris::types {
    struct jvalue {
        using jarray = std::vector<jtype>;
        using jobject = tsl::robin_map<std::string, jtype>;

        template <typename T>
        using enable_if_integer_t =
            std::enable_if_t<std::is_integral<T>::value && !std::is_same_v<T, bool> && !std::is_convertible_v<T, const char*>, int>;

        template <typename T>
        using enable_if_floating_t = std::enable_if_t<std::is_floating_point<T>::value, int>;

      private:
        std::variant<std::monostate, bool, int64_t, double, std::string, jobject, jarray> value_;

      public:
        // --- constructors ---
        inline explicit jvalue() noexcept : value_(std::monostate{}) {}
        inline jvalue(const jvalue& other) { *this = other; }
        inline jvalue(jvalue&& other) noexcept = default;

        inline explicit jvalue(bool value) noexcept : value_(value) {}

        inline explicit jvalue(const std::string& value) : value_(value) {}
        inline explicit jvalue(std::string&& value) noexcept : value_(std::move(value)) {}
        inline explicit jvalue(const char* value) : value_(std::string(value)) {}

        template <typename T, enable_if_integer_t<T> = 0>
        inline explicit jvalue(T value) noexcept : value_(static_cast<int64_t>(value)) {}

        template <typename T, enable_if_floating_t<T> = 0>
        inline explicit jvalue(T value) noexcept : value_(static_cast<double>(value)) {}

        inline jvalue(const jobject&) = delete;
        inline explicit jvalue(jobject&& value) noexcept : value_(std::move(value)) {}

        inline jvalue(const jarray&) = delete;
        inline explicit jvalue(jarray&& value) noexcept : value_(std::move(value)) {}

        // --- accessors ---
        std::optional<bool> try_as_bool() const noexcept;
        bool as_bool() const;

        const std::string* try_as_string() const noexcept;
        const std::string& as_string() const;

        std::optional<int> try_as_int() const noexcept;
        int as_int() const;

        std::optional<short> try_as_short() const noexcept;
        short as_short() const;

        std::optional<long> try_as_long() const noexcept;
        long as_long() const;

        std::optional<float> try_as_float() const noexcept;
        float as_float() const;

        std::optional<double> try_as_double() const noexcept;
        double as_double() const;

        const jobject* try_as_object() const noexcept;
        const jobject& as_object() const;

        const jarray* try_as_array() const noexcept;
        const jarray& as_array() const;

        // --- helpers ---
        const jvalue* try_get(const std::string& key) const noexcept;
        const jvalue& get(const std::string& key) const;

        const jvalue* try_get(size_t idx) const noexcept;
        const jvalue& get(size_t idx) const;

        jvalue& insert(const std::string& key);
        jvalue& insert(size_t idx);

        // --- utilities ---
        std::unique_ptr<jvalue> clone() const;

        constexpr bool is_null() const noexcept;
        constexpr bool is_bool() const noexcept;
        constexpr bool is_string() const noexcept;

        constexpr bool is_integer() const noexcept;
        constexpr bool is_floating() const noexcept;
        constexpr bool is_number() const noexcept;

        constexpr bool is_array() const noexcept;
        constexpr bool is_object() const noexcept;

        // --- operators (deep-copy + move) ---
        jvalue& operator=(const jvalue& other);
        jvalue& operator=(jvalue&& other) noexcept = default;

        // --- operators (equality) ---
        bool operator==(const jvalue& other) const noexcept;
        bool operator!=(const jvalue& other) const noexcept;

        // --- operators (accessors) ---
        jvalue& operator[](const std::string& key);
        const jvalue& operator[](const std::string& key) const;

        jvalue& operator[](size_t idx);
        const jvalue& operator[](size_t idx) const;
    };
}

#include "jvalue.inl"
