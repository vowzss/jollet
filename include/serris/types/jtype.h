#pragma once

#include <type_traits>

namespace serris::types {
    struct jvalue;

    struct jtype {
        static constexpr size_t INLINE_CAPACITY = sizeof(void*) * 2;

        std::aligned_storage_t<INLINE_CAPACITY, alignof(void*)> buffer;
        bool inlined = false;

        // --- constructors ---
        jtype() noexcept = default;
        jtype(const jtype& other);
        jtype(jtype&& other) noexcept;

        // ---  ---
        jvalue* get() noexcept;
        const jvalue* get() const noexcept;

        jtype clone() const;

        // --- operators ---
        jtype& operator=(const jtype& other);
        jtype& operator=(jtype&& other) noexcept;
    };
}

#include "jtype.inl"