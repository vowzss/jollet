#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <tsl/robin_map.h>

namespace serris::types {

    struct jvalue {
        using jarray = std::vector<std::unique_ptr<jvalue>>;
        using jobject = tsl::robin_map<std::string, std::unique_ptr<jvalue>>;

        template <typename T>
        using enable_if_arithmetic_t = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, int>;

        template <typename T>
        using disable_if_arithmetic_t = std::enable_if_t<!std::is_arithmetic_v<T>, int>;

      private:
        std::variant<std::monostate, bool, int64_t, double, std::string, jobject, jarray> value_;

      public:
        explicit jvalue() : value_(std::monostate{}) {}
        explicit jvalue(bool val) : value_(val) {}

        // --- string type ctor ---
        explicit jvalue(const std::string& s) : value_(s) {}
        explicit jvalue(std::string&& s) : value_(std::move(s)) {}

        // --- number types ctor ---
        template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
        explicit jvalue(T val) {
            if constexpr (std::is_floating_point_v<T>) {
                value_ = static_cast<double>(val);
            } else {
                value_ = static_cast<int64_t>(val);
            }
        }

        // --- object type ctor ---
        explicit jvalue(const jobject& val) : value_(val) {}
        explicit jvalue(jobject&& val) : value_(std::move(val)) {}

        // --- array type ctor ---
        explicit jvalue(const jarray& val) : value_(val) {}
        explicit jvalue(jarray&& val) : value_(std::move(val)) {}

        // --- accessors ---
        template <typename T>
        std::optional<T> try_as() const {
            if constexpr (std::is_arithmetic_v<T>) {
                if (const int64_t* v = std::get_if<int64_t>(&value_)) {
                    return static_cast<T>(*v);
                }
                if (const double* v = std::get_if<double>(&value_)) {
                    return static_cast<T>(*v);
                }
                return std::nullopt;
            } else if constexpr (std::is_same_v<T, std::string>) {
                if (const std::string* v = std::get_if<std::string>(&value_)) {
                    return *v;
                }
                return std::nullopt;
            } else if constexpr (std::is_same_v<T, jarray>) {
                if (const jarray* v = std::get_if<jarray>(&value_)) {
                    return *v;
                }
                return std::nullopt;
            } else if constexpr (std::is_same_v<T, jobject>) {
                if (const jobject* v = std::get_if<jobject>(&value_)) {
                    return *v;
                }
                return std::nullopt;
            } else {
                static_assert(false, "type is not supported in jvalue");
            }
        }

        template <typename T, enable_if_arithmetic_t<T> = 0>
        T as() const {
            if (auto val = try_as<T>()) return *val;
            throw std::bad_variant_access();
        }

        // --- accessors (aliases for common types) ---
        std::optional<bool> try_as_bool() const {
            if (const bool* val = std::get_if<bool>(&value_)) return *val;
            return std::nullopt;
        }
        bool as_bool() const {
            if (std::optional<bool> val = try_as_bool()) return *val;
            throw std::bad_variant_access();
        }

        const std::string* try_as_string() const {
            if (const std::string* val = std::get_if<std::string>(&value_)) return val;
            return nullptr;
        }
        const std::string& as_string() const {
            if (const std::string* val = try_as_string()) return *val;
            throw std::bad_variant_access();
        }

        std::optional<int> try_as_int() const { return try_as<int>(); }
        int as_int() const { return as<int>(); }

        std::optional<short> try_as_short() const { return try_as<short>(); }
        short as_short() const { return as<short>(); }

        std::optional<long> try_as_long() const { return try_as<long>(); }
        long as_long() const { return as<long>(); }

        std::optional<float> try_as_float() const { return try_as<float>(); }
        float as_float() const { return as<float>(); }

        std::optional<double> try_as_double() const { return try_as<double>(); }
        double as_double() const { return as<double>(); }

        const jobject* try_as_object() const {
            if (const jobject* val = std::get_if<jobject>(&value_)) return val;
            return nullptr;
        }
        const jobject& as_object() const {
            if (const jobject* val = try_as_object()) return *val;
            throw std::bad_variant_access();
        }

        const jarray* try_as_array() const {
            if (const jarray* val = std::get_if<jarray>(&value_)) return val;
            return nullptr;
        }
        const jarray& as_array() const {
            if (const jarray* val = try_as_array()) return *val;
            throw std::bad_variant_access();
        }

        // --- equality operators ---
        bool operator==(const jvalue& other) const { return value_ == other.value_; }
        bool operator!=(const jvalue& other) const { return !(*this == other); }

        // --- helpers ---
        const jvalue* get(size_t idx) const {
            if (const jarray* arr = std::get_if<jarray>(&value_)) {
                if (idx < arr->size()) {
                    return arr->at(idx).get();
                }
            }
            return nullptr;
        }

        jvalue& insert(size_t idx) {
            if (std::holds_alternative<std::monostate>(value_)) {
                value_ = jarray{};
            }

            if (jarray* arr = std::get_if<jarray>(&value_)) {
                if (idx >= arr->size()) {
                    arr->resize(idx + 1);

                    if (!(*arr)[idx]) {
                        (*arr)[idx] = std::make_unique<jvalue>();
                    }

                    return *(*arr)[idx];
                }

                throw std::bad_variant_access();
            }
        }

        // --- utilities ---
        constexpr bool is_null() const { return std::holds_alternative<std::monostate>(value_); }
        constexpr bool is_bool() const { return std::holds_alternative<bool>(value_); }
        constexpr bool is_string() const { return std::holds_alternative<std::string>(value_); }

        constexpr bool is_integer() const { return std::holds_alternative<int64_t>(value_); }
        constexpr bool is_floating() const { return std::holds_alternative<double>(value_); }
        constexpr bool is_number() const { return is_integer() || is_floating(); }

        constexpr bool is_array() const { return std::holds_alternative<jarray>(value_); }
        constexpr bool is_object() const { return std::holds_alternative<jobject>(value_); }
    };
}
