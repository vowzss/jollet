#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace serris::types {
    struct jvalue {
        using jobject = std::unordered_map<std::string, jvalue>;
        using jarray = std::vector<jvalue>;

        using number_t = std::variant<int64_t, double>;

        template <typename T>
        using enable_if_arithmetic_t = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, int>;

        template <typename T>
        using disable_if_arithmetic_t = std::enable_if_t<!std::is_arithmetic_v<T>, int>;

      private:
        std::variant<std::monostate, bool, number_t, std::string, jobject, jarray> value_;

      public:
        explicit jvalue() : value_(std::monostate{}) {}
        explicit jvalue(bool val) : value_(val) {}

        // --- number types ctor ---
        template <typename T, enable_if_arithmetic_t<T> = 0>
        explicit jvalue(T val) {
            if constexpr (std::is_floating_point_v<T>) {
                value_ = number_t(static_cast<double>(val));
            } else {
                value_ = number_t(static_cast<int64_t>(val));
            }
        }

        // --- string-like types ctor ---
        template <typename T, std::enable_if_t<std::is_convertible_v<T, std::string>, int> = 0>
        explicit jvalue(T&& val) : value_(std::forward<T>(val)) {}

        // --- object type (copy + move) ctor ---
        explicit jvalue(const jobject& val) : value_(val) {}
        explicit jvalue(jobject&& val) : value_(std::move(val)) {}

        // --- array type (copy + move) ctor ---
        explicit jvalue(const jarray& val) : value_(val) {}
        explicit jvalue(jarray&& val) : value_(std::move(val)) {}

        // --- accessors (arithmetic) ---
        template <typename T, enable_if_arithmetic_t<T> = 0>
        std::optional<T> try_as() const {
            if (const number_t* val = std::get_if<number_t>(&value_)) {
                return std::visit([](auto&& v) { return static_cast<T>(v); }, *val);
            }

            return std::nullopt;
        }

        template <typename T, enable_if_arithmetic_t<T> = 0>
        T as() const {
            if (const std::optional<T> val = try_as<T>()) {
                return *val;
            }

            throw std::bad_variant_access();
        }

        // --- accessors (non-arithmetic) ---
        template <typename T, disable_if_arithmetic_t<T> = 0>
        const T* try_as() const {
            return std::get_if<T>(&value_);
        }

        template <typename T, disable_if_arithmetic_t<T> = 0>
        const T& as() const {
            if (const T* v = try_as<T>()) {
                return *v;
            }

            throw std::bad_variant_access();
        }

        // --- accessors (typed aliases for common types) ---
        bool as_bool() const {
            if (auto val = try_as_bool()) {
                return *val;
            }

            throw std::bad_variant_access();
        }
        std::optional<bool> try_as_bool() const {
            if (const bool* val = std::get_if<bool>(&value_)) {
                return *val;
            }

            return std::nullopt;
        }

        int as_int() const { return as<int>(); }
        std::optional<int> try_as_int() const { return try_as<int>(); }

        short as_short() const { return as<short>(); }
        std::optional<short> try_as_short() const { return try_as<short>(); }

        long as_long() const { return as<long>(); }
        std::optional<long> try_as_long() const { return try_as<long>(); }

        float as_float() const { return as<float>(); }
        std::optional<float> try_as_float() const { return try_as<float>(); }

        double as_double() const { return as<double>(); }
        std::optional<double> try_as_double() const { return try_as<double>(); }

        const std::string& as_string() const { return as<std::string>(); }
        const std::string* try_as_string() const { return try_as<std::string>(); }

        const jobject& as_object() const { return as<jobject>(); }
        const jobject* try_as_object() const { return try_as<jobject>(); }

        const jarray& as_array() const { return as<jarray>(); }
        const jarray* try_as_array() const { return try_as<jarray>(); }

        // --- equality operators ---
        bool operator==(const jvalue& other) const { return value_ == other.value_; }
        bool operator!=(const jvalue& other) const { return !(*this == other); }

        // --- operator[] for objects ---
        const jvalue& operator[](const std::string& key) const {
            if (const jobject* v = std::get_if<jobject>(&value_)) {
                return v->at(key);
            }

            throw std::bad_variant_access();
        }

        jvalue& operator[](const std::string& key) {
            if (std::holds_alternative<std::monostate>(value_)) {
                value_ = jobject{};
            }

            if (jobject* v = std::get_if<jobject>(&value_)) {
                return (*v)[key];
            }

            throw std::bad_variant_access();
        }

        jvalue& operator[](std::string&& key) {
            if (std::holds_alternative<std::monostate>(value_)) {
                value_ = jobject{};
            }

            if (jobject* v = std::get_if<jobject>(&value_)) {
                return (*v)[std::move(key)];
            }

            throw std::bad_variant_access();
        }

        // --- operator[] for arrays ---
        const jvalue& operator[](size_t idx) const {
            if (const jarray* v = std::get_if<jarray>(&value_)) {
                return v->at(idx);
            }

            throw std::bad_variant_access();
        }

        jvalue& operator[](size_t idx) {
            if (std::holds_alternative<std::monostate>(value_)) {
                value_ = jarray{};
            }

            if (jarray* v = std::get_if<jarray>(&value_)) {
                if (idx >= v->size()) {
                    v->resize(idx + 1);
                }

                return (*v)[idx];
            }

            throw std::bad_variant_access();
        }

        // --- utilities ---
        constexpr bool is_null() const { return std::holds_alternative<std::monostate>(value_); }
    };
}
