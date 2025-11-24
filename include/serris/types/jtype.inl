#pragma once

#include <memory>

namespace serris::types {
    // --- constructors ---
    inline jtype::jtype(const jtype& other) {
        std::memcpy(&buffer, &other.buffer, INLINE_CAPACITY);
        inlined = other.inlined;
    }

    inline jtype::jtype(jtype&& other) noexcept {
        std::memcpy(&buffer, &other.buffer, INLINE_CAPACITY);
        inlined = other.inlined;
    }

    // ---  ---
    inline jtype jtype::clone() const {
        jtype copy;
        copy = *this;
        return copy;
    }

    inline jvalue* jtype::get() noexcept {
        return inlined ? reinterpret_cast<jvalue*>(&buffer) : nullptr;
    }

    inline const jvalue* jtype::get() const noexcept {
        return inlined ? reinterpret_cast<const jvalue*>(&buffer) : nullptr;
    }

    // --- operators ---
    inline jtype& jtype::operator=(const jtype& other) {
        if (this != &other) {
            std::memcpy(&buffer, &other.buffer, INLINE_CAPACITY);
            inlined = other.inlined;
        }

        return *this;
    }

    inline jtype& jtype::operator=(jtype&& other) noexcept {
        if (this != &other) {
            std::memcpy(&buffer, &other.buffer, INLINE_CAPACITY);
            inlined = other.inlined;
        }

        return *this;
    }
}