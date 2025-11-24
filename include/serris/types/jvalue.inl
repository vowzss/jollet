#pragma once

#include <optional>

namespace serris::types {
    namespace {
        jvalue::jarray copy_array(const jvalue::jarray& value) {
            jvalue::jarray copy{};
            copy.reserve(value.size());
            for (auto& ptr : value) {
                copy.push_back(ptr.clone());
            }
            return copy;
        }

        jvalue::jobject copy_object(const jvalue::jobject& value) {
            jvalue::jobject copy{};
            for (auto& [k, ptr] : value) {
                copy.emplace(k, ptr.clone());
            }
            return copy;
        }
    }

    // --- accessors ---
    inline std::optional<bool> jvalue::try_as_bool() const noexcept {
        if (const bool* val = std::get_if<bool>(&value_)) return *val;
        return std::nullopt;
    }
    inline bool jvalue::as_bool() const {
        return std::get<bool>(value_);
    }

    inline const std::string* jvalue::try_as_string() const noexcept {
        return std::get_if<std::string>(&value_);
    }
    inline const std::string& jvalue::as_string() const {
        return std::get<std::string>(value_);
    }

    inline std::optional<int> jvalue::try_as_int() const noexcept {
        if (const int64_t* val = std::get_if<int64_t>(&value_)) return static_cast<int>(*val);
        return std::nullopt;
    }
    inline int jvalue::as_int() const {
        return static_cast<int>(std::get<int64_t>(value_));
    }

    inline std::optional<short> jvalue::try_as_short() const noexcept {
        if (const int64_t* val = std::get_if<int64_t>(&value_)) return static_cast<short>(*val);
        return std::nullopt;
    }
    inline short jvalue::as_short() const {
        return static_cast<short>(std::get<int64_t>(value_));
    }

    inline std::optional<long> jvalue::try_as_long() const noexcept {
        if (const int64_t* val = std::get_if<int64_t>(&value_)) return static_cast<long>(*val);
        return std::nullopt;
    }
    inline long jvalue::as_long() const {
        return static_cast<long>(std::get<int64_t>(value_));
    }

    inline std::optional<float> jvalue::try_as_float() const noexcept {
        if (const double* val = std::get_if<double>(&value_)) return static_cast<float>(*val);
        return std::nullopt;
    }
    inline float jvalue::as_float() const {
        return static_cast<float>(std::get<double>(value_));
    }

    inline std::optional<double> jvalue::try_as_double() const noexcept {
        if (const double* val = std::get_if<double>(&value_)) return *val;
        return std::nullopt;
    }
    inline double jvalue::as_double() const {
        return std::get<double>(value_);
    }

    inline const jvalue::jobject* jvalue::try_as_object() const noexcept {
        return std::get_if<jobject>(&value_);
    }
    inline const jvalue::jobject& jvalue::as_object() const {
        return std::get<jobject>(value_);
    }

    inline const jvalue::jarray* jvalue::try_as_array() const noexcept {
        return std::get_if<jarray>(&value_);
    }
    inline const jvalue::jarray& jvalue::as_array() const {
        return std::get<jarray>(value_);
    }

    // --- helpers ---
    inline const jvalue* jvalue::try_get(const std::string& key) const noexcept {
        if (!is_object()) return nullptr;

        const auto& obj = std::get<jobject>(value_);
        auto it = obj.find(key);
        if (it == obj.end()) return nullptr;

        return it->second.get();
    }

    inline const jvalue& jvalue::get(const std::string& key) const {
        if (!is_object()) {
            throw std::runtime_error("jvalue::get(key) called on non-object type");
        }

        const jobject& obj = std::get<jobject>(value_);
        auto it = obj.find(key);
        const jvalue* val = (it != obj.end()) ? it->second.get() : nullptr;
        if (!val) {
            throw std::out_of_range("jvalue::get(key) key not found: " + key);
        }

        return *val;
    }

    inline const jvalue* jvalue::try_get(size_t idx) const noexcept {
        if (!is_array()) return nullptr;

        const auto& arr = std::get<jarray>(value_);
        return (idx < arr.size()) ? arr[idx].get() : nullptr;
    }

    inline const jvalue& jvalue::get(size_t idx) const {
        if (!is_array()) {
            throw std::runtime_error("jvalue::get(idx) called on non-array type");
        }

        const jarray& arr = std::get<jarray>(value_);
        const jvalue* val = (idx < arr.size()) ? arr[idx].get() : nullptr;
        if (!val) {
            throw std::out_of_range("jvalue::get(idx) index out of range");
        }

        return *val;
    }

    inline jvalue& jvalue::insert(const std::string& key) {
        if (!is_object()) {
            throw std::runtime_error("insert(key) called on non-object type");
        }

        jobject& obj = std::get<jobject>(value_);
        auto& entry = obj[key];

        if (!entry.get()) {
            new (&entry.buffer) jvalue();
            entry.inlined = true;
        }

        return *entry.get();
    }

    inline jvalue& jvalue::insert(size_t idx) {
        if (!is_array()) {
            throw std::runtime_error("insert(idx) called on non-array type");
        }

        jarray& arr = std::get<jarray>(value_);
        if (idx >= arr.size()) arr.resize(idx + 1);

        auto& entry = arr[idx];
        if (!entry.get()) {
            new (&entry.buffer) jvalue();
            entry.inlined = true;
        }

        return *entry.get();
    }

    // --- utilities ---
    inline std::unique_ptr<jvalue> jvalue::clone() const {
        return std::make_unique<jvalue>(*this);
    }

    inline constexpr bool jvalue::is_null() const noexcept {
        return std::holds_alternative<std::monostate>(value_);
    }
    inline constexpr bool jvalue::is_bool() const noexcept {
        return std::holds_alternative<bool>(value_);
    }
    inline constexpr bool jvalue::is_string() const noexcept {
        return std::holds_alternative<std::string>(value_);
    }

    inline constexpr bool jvalue::is_integer() const noexcept {
        return std::holds_alternative<int64_t>(value_);
    }
    inline constexpr bool jvalue::is_floating() const noexcept {
        return std::holds_alternative<double>(value_);
    }
    inline constexpr bool jvalue::is_number() const noexcept {
        return is_integer() || is_floating();
    }

    inline constexpr bool jvalue::is_array() const noexcept {
        return std::holds_alternative<jarray>(value_);
    }
    inline constexpr bool jvalue::is_object() const noexcept {
        return std::holds_alternative<jobject>(value_);
    }

    // --- operators (deep-copy + move) ---
    inline jvalue& jvalue::operator=(const jvalue& other) {
        if (this == &other) return *this;

        value_ = std::visit(
            [](auto&& v) -> decltype(value_) {
                using T = std::decay_t<decltype(v)>;

                if constexpr (std::is_same_v<T, jarray>) {
                    jvalue::jarray copy;
                    copy.reserve(v.size());
                    for (const auto& e : v) {
                        copy.push_back(e.clone());
                    }
                    return copy;
                } else if constexpr (std::is_same_v<T, jobject>) {
                    jvalue::jobject copy;
                    copy.reserve(v.size());
                    for (const auto& [k, ptr] : v) {
                        copy.emplace(k, ptr.clone());
                    }
                    return copy;
                } else {
                    return v;
                }
            },
            other.value_);

        return *this;
    }

    // --- operators (equality) ---
    inline bool jvalue::operator==(const jvalue& other) const noexcept {
        if (value_.index() != other.value_.index()) return false;

        return std::visit(
            [&](const auto& lhs) -> bool {
                using T = std::decay_t<decltype(lhs)>;

                if constexpr (std::is_same_v<T, jarray>) {
                    const auto& rhs = std::get<jarray>(other.value_);
                    if (lhs.size() != rhs.size()) return false;

                    for (size_t i = 0; i < lhs.size(); ++i) {
                        const jvalue* lv = lhs[i].get();
                        const jvalue* rv = rhs[i].get();

                        if (lv && rv) {
                            if (!(*lv == *rv)) return false;
                        } else if (lv || rv) {
                            return false; // one is null, the other isn't
                        }
                    }
                    return true;

                } else if constexpr (std::is_same_v<T, jobject>) {
                    const auto& rhs = std::get<jobject>(other.value_);
                    if (lhs.size() != rhs.size()) return false;

                    for (const auto& [k, lp] : lhs) {
                        auto it = rhs.find(k);
                        if (it == rhs.end()) return false;

                        const jvalue* lv = lp.get();
                        const jvalue* rv = it->second.get();

                        if (lv && rv) {
                            if (!(*lv == *rv)) return false;
                        } else if (lv || rv) {
                            return false;
                        }
                    }
                    return true;

                } else {
                    return lhs == std::get<T>(other.value_);
                }
            },
            value_);
    }
    inline bool jvalue::operator!=(const jvalue& other) const noexcept {
        return !(*this == other);
    }

    // --- operators (accessors) ---
    inline jvalue& jvalue::operator[](const std::string& key) {
        return insert(key);
    }

    inline const jvalue& jvalue::operator[](const std::string& key) const {
        return get(key);
    }

    inline jvalue& jvalue::operator[](size_t idx) {
        return insert(idx);
    }

    inline const jvalue& jvalue::operator[](size_t idx) const {
        return get(idx);
    }
}
