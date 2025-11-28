#pragma once

namespace serris {
    // ---  ---
    inline JValue* JType::get() noexcept {
        return inlined ? reinterpret_cast<JValue*>(&buffer) : nullptr;
    }

    inline const JValue* JType::get() const noexcept {
        return inlined ? reinterpret_cast<const JValue*>(&buffer) : nullptr;
    }

    inline void JType::swap(JType& other) noexcept {
        if (this == &other) return;

        std::swap(inlined, other.inlined);

        std::aligned_storage_t<JType::INLINE_CAPACITY, alignof(void*)> temp;
        std::memcpy(&temp, &buffer, JType::INLINE_CAPACITY);
        std::memcpy(&buffer, &other.buffer, JType::INLINE_CAPACITY);
        std::memcpy(&other.buffer, &temp, JType::INLINE_CAPACITY);
    }
}