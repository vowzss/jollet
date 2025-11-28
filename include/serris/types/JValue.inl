#pragma once

namespace serris {
    // --- constructors ---
    inline JValue::JValue(const JValue& other) {
        std::visit(
            [this](auto&& val) {
                using T = std::decay_t<decltype(val)>;

                if constexpr (std::is_same_v<T, JObject>) {
                    JObject copy;
                    copy.reserve(val.size());

                    for (auto& [k, entry] : val) {
                        JType e;
                        if (auto* jv = entry.get()) {
                            new (&e.buffer) JValue(*jv);
                            e.inlined = true;
                        }
                        copy.emplace(k, std::move(e));
                    }

                    value = std::move(copy);
                } else if constexpr (std::is_same_v<T, JArray>) {
                    JArray copy;
                    copy.reserve(val.size());

                    for (auto& entry : val) {
                        JType e;
                        if (auto* jv = entry.get()) {
                            new (&e.buffer) JValue(*jv);
                            e.inlined = true;
                        }
                        copy.push_back(std::move(e));
                    }

                    value = std::move(copy);
                } else {
                    value = val;
                }
            },
            other.value);
    }

    inline JValue::JValue(JValue&& other) noexcept {
        std::visit(
            [this](auto&& val) {
                using T = std::decay_t<decltype(val)>;

                if constexpr (std::is_same_v<T, JObject>) {
                    JObject moved;
                    moved.reserve(val.size());

                    for (auto& [k, entry] : val) {
                        JType e;
                        if (auto* jv = entry.get()) {
                            new (&e.buffer) JValue(std::move(*jv));
                            e.inlined = true;
                        }
                        moved.emplace(k, std::move(e));
                    }

                    value = std::move(moved);
                } else if constexpr (std::is_same_v<T, JArray>) {
                    JArray moved;
                    moved.reserve(val.size());

                    for (auto& entry : val) {
                        JType e;
                        if (auto* jv = entry.get()) {
                            new (&e.buffer) JValue(std::move(*jv));
                            e.inlined = true;
                        }
                        moved.push_back(std::move(e));
                    }

                    value = std::move(moved);
                } else {
                    value = val;
                }
            },
            other.value);
    }

    // --- helpers (common types) ---
    inline std::optional<bool> JValue::tryAsBool() const noexcept {
        if (const bool* val = std::get_if<bool>(&value)) return *val;
        return std::nullopt;
    }
    inline bool JValue::asBool() const {
        return std::get<bool>(value);
    }

    inline const std::string* JValue::tryAsString() const noexcept {
        return std::get_if<std::string>(&value);
    }
    inline const std::string& JValue::asString() const {
        return std::get<std::string>(value);
    }

    inline std::optional<int> JValue::tryAsInt() const noexcept {
        if (const int64_t* val = std::get_if<int64_t>(&value)) return static_cast<int>(*val);
        return std::nullopt;
    }
    inline int JValue::asInt() const {
        return static_cast<int>(std::get<int64_t>(value));
    }

    inline std::optional<short> JValue::tryAsShort() const noexcept {
        if (const int64_t* val = std::get_if<int64_t>(&value)) return static_cast<short>(*val);
        return std::nullopt;
    }
    inline short JValue::asShort() const {
        return static_cast<short>(std::get<int64_t>(value));
    }

    inline std::optional<long> JValue::tryAsLong() const noexcept {
        if (const int64_t* val = std::get_if<int64_t>(&value)) return static_cast<long>(*val);
        return std::nullopt;
    }
    inline long JValue::asLong() const {
        return static_cast<long>(std::get<int64_t>(value));
    }

    inline std::optional<float> JValue::tryAsFloat() const noexcept {
        if (const double* val = std::get_if<double>(&value)) return static_cast<float>(*val);
        return std::nullopt;
    }
    inline float JValue::asFloat() const {
        return static_cast<float>(std::get<double>(value));
    }

    inline std::optional<double> JValue::tryAsDouble() const noexcept {
        if (const double* val = std::get_if<double>(&value)) return *val;
        return std::nullopt;
    }
    inline double JValue::asDouble() const {
        return std::get<double>(value);
    }

    // --- helpers (object) ---
    inline const JValue* JValue::find(const std::string& key) const noexcept {
        if (!isObject()) return nullptr;

        const JObject& obj = std::get<JObject>(value);
        auto it = obj.find(key);
        if (it == obj.end()) return nullptr;

        return it->second.get();
    }

    inline JValue::JObjectEntries JValue::entries() const {
        if (!isObject()) {
            throw std::runtime_error("entries() called on non-object type");
        }

        return JObjectEntries{std::get<JObject>(value)};
    }

    // --- helpers (array) ---
    inline const JValue* JValue::find(size_t idx) const noexcept {
        if (!isArray()) return nullptr;

        const JArray& arr = std::get<JArray>(value);
        return (idx < arr.size()) ? arr[idx].get() : nullptr;
    }

    inline JValue& JValue::emplace(JValue&& val) {
        if (!isArray()) {
            throw std::runtime_error("emplace(JValue&&) called on non-array type");
        }

        JArray& arr = std::get<JArray>(value);

        JType entry;
        new (&entry.buffer) JValue(std::move(val));
        entry.inlined = true;

        arr.push_back(std::move(entry));

        return *arr.back().get();
    }

    inline JValue::JArrayElements JValue::elements() const {
        if (!isArray()) {
            throw std::runtime_error("elements() called on non-array type");
        }
        return JValue::JArrayElements(std::get<JArray>(value));
    }

    // --- utilities ---
    inline std::unique_ptr<JValue> JValue::clone() const {
        auto copy = std::make_unique<JValue>();

        std::visit(
            [&copy](auto&& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, JObject>) {
                    JObject obj;
                    obj.reserve(val.size());

                    for (auto& [k, t] : val) {
                        if (auto* jv = t.get()) {
                            JType newEntry;
                            new (&newEntry.buffer) JValue(*jv->clone());
                            newEntry.inlined = true;

                            obj.emplace(k, std::move(newEntry));
                        }
                    }

                    copy->value = std::move(obj);
                } else if constexpr (std::is_same_v<T, JArray>) {
                    JArray arr;
                    arr.reserve(val.size());

                    for (auto& t : val) {
                        if (auto* jv = t.get()) {
                            JType newEntry;
                            new (&newEntry.buffer) JValue(*jv->clone());
                            newEntry.inlined = true;

                            arr.push_back(std::move(newEntry));
                        }
                    }

                    copy->value = std::move(arr);
                } else {
                    copy->value = val;
                }
            },
            value);

        return copy;
    }

    inline constexpr bool JValue::isNull() const noexcept {
        return std::holds_alternative<std::monostate>(value);
    }
    inline constexpr bool JValue::isBool() const noexcept {
        return std::holds_alternative<bool>(value);
    }
    inline constexpr bool JValue::isString() const noexcept {
        return std::holds_alternative<std::string>(value);
    }

    inline constexpr bool JValue::isInteger() const noexcept {
        return std::holds_alternative<int64_t>(value);
    }
    inline constexpr bool JValue::isFloating() const noexcept {
        return std::holds_alternative<double>(value);
    }
    inline constexpr bool JValue::isNumber() const noexcept {
        return isInteger() || isFloating();
    }

    inline constexpr bool JValue::isArray() const noexcept {
        return std::holds_alternative<JArray>(value);
    }
    inline constexpr bool JValue::isObject() const noexcept {
        return std::holds_alternative<JObject>(value);
    }

    // --- operators (deep-copy + move) ---
    inline JValue& JValue::operator=(const JValue& other) {
        if (this == &other) return *this;

        this->~JValue();
        new (this) JValue(other);

        return *this;
    }

    inline JValue& JValue::operator=(JValue&& other) noexcept {
        if (this == &other) return *this;

        this->~JValue();
        new (this) JValue(std::move(other));

        return *this;
    }

    // --- operators (equality) ---
    inline bool JValue::operator==(const JValue& other) const noexcept {
        if (value.index() != other.value.index()) return false;

        return std::visit(
            [&](const auto& lhs) -> bool {
                using T = std::decay_t<decltype(lhs)>;

                if constexpr (std::is_same_v<T, JArray>) {
                    const auto& rhs = std::get<JArray>(other.value);
                    if (lhs.size() != rhs.size()) return false;

                    for (size_t i = 0; i < lhs.size(); ++i) {
                        const JValue* lv = lhs[i].get();
                        const JValue* rv = rhs[i].get();

                        if (lv && rv) {
                            if (!(*lv == *rv)) return false;
                        } else if (lv || rv) {
                            return false;
                        }
                    }
                    return true;

                } else if constexpr (std::is_same_v<T, JObject>) {
                    const auto& rhs = std::get<JObject>(other.value);
                    if (lhs.size() != rhs.size()) return false;

                    for (const auto& [k, lp] : lhs) {
                        auto it = rhs.find(k);
                        if (it == rhs.end()) return false;

                        const JValue* lv = lp.get();
                        const JValue* rv = it->second.get();

                        if (lv && rv) {
                            if (!(*lv == *rv)) return false;
                        } else if (lv || rv) {
                            return false;
                        }
                    }
                    return true;

                } else {
                    return lhs == std::get<T>(other.value);
                }
            },
            value);
    }
    inline bool JValue::operator!=(const JValue& other) const noexcept {
        return !(*this == other);
    }

    // --- operators (accessors) ---
    inline JValue& JValue::operator[](const std::string& key) {
        if (!isObject()) {
            if (!isNull()) {
                throw std::runtime_error("operator[](string&) called on non-object type");
            }

            value = JObject{};
        }

        JObject& obj = std::get<JObject>(value);
        JType& entry = obj[key];

        if (!entry.get()) {
            new (&entry.buffer) JValue();
            entry.inlined = true;
        }

        return *entry.get();
    }

    inline const JValue& JValue::operator[](const std::string& key) const {
        if (!isObject()) {
            throw std::runtime_error("at(string&) called on non-object type");
        }

        const JObject& obj = std::get<JObject>(value);
        auto it = obj.find(key);
        const JValue* val = (it != obj.end()) ? it->second.get() : nullptr;
        if (!val) {
            throw std::out_of_range("at(string&) key not found: " + key);
        }

        return *val;
    }

    inline JValue& JValue::operator[](size_t idx) {
        if (!isArray()) {
            if (!isNull()) {
                throw std::runtime_error("operator[](size_t) called on non-array type");
            }

            value = JObject{};
        }

        JArray& arr = std::get<JArray>(value);
        if (idx >= arr.size()) arr.resize(idx + 1);

        JType& entry = arr[idx];
        if (!entry.get()) {
            new (&entry.buffer) JValue();
            entry.inlined = true;
        }

        return *entry.get();
    }

    inline const JValue& JValue::operator[](size_t idx) const {
        if (!isArray()) {
            throw std::runtime_error("at(size_t) called on non-array type");
        }

        const JArray& arr = std::get<JArray>(value);
        const JValue* val = (idx < arr.size()) ? arr[idx].get() : nullptr;
        if (!val) {
            throw std::out_of_range("at(size_t) index out of range");
        }

        return *val;
    }

    // --- iterators ---
    struct JValue::JObjectEntries::Iterator {
      private:
        using It = JValue::JObject::const_iterator;
        It it;

      public:
        Iterator(It i) : it(i) {}

        JObjectEntry operator*() const {
            const JType& t = it->second;
            return JObjectEntry{it->first, *t.get()};
        }

        Iterator& operator++() {
            ++it;
            return *this;
        }

        bool operator!=(const Iterator& other) const { return it != other.it; }
    };

    inline JValue::JObjectEntries::Iterator JValue::JObjectEntries::begin() const {
        return Iterator(obj.begin());
    }

    inline JValue::JObjectEntries::Iterator JValue::JObjectEntries::end() const {
        return Iterator(obj.end());
    }

    struct JValue::JArrayElements::Iterator {
      private:
        using It = JValue::JArray::const_iterator;
        It it;
        size_t index;

      public:
        Iterator(It i, size_t idx) : it(i), index(idx) {}

        JArrayElement operator*() const {
            const JType& t = *it;
            return JArrayElement{index, *t.get()};
        }

        Iterator& operator++() {
            ++it;
            ++index;
            return *this;
        }

        bool operator!=(const Iterator& other) const { return it != other.it; }
    };

    inline JValue::JArrayElements::Iterator JValue::JArrayElements::begin() const {
        return Iterator(arr.begin(), 0);
    }

    inline JValue::JArrayElements::Iterator JValue::JArrayElements::end() const {
        return Iterator(arr.end(), arr.size());
    }
}
