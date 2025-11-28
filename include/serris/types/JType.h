#pragma once

#include <type_traits>

namespace serris {
    struct JValue;

    struct JType {
        static constexpr size_t INLINE_CAPACITY = sizeof(void*) * 2;

        std::aligned_storage_t<INLINE_CAPACITY, alignof(void*)> buffer;
        bool inlined = false;

        // --- constructors ---
        JType() = default;
        JType(const JType& other) = default;
        JType(JType&& other) = default;

        // ---  ---
        JValue* get() noexcept;
        const JValue* get() const noexcept;

        void swap(JType& other) noexcept;

        // --- operators ---
        JType& operator=(const JType&) = default;
        JType& operator=(JType&&) = default;
    };
}

#include "JType.inl"