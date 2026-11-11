// SPDX-FileCopyrightText: 2026 Alexey Tkachenko
// SPDX-License-Identifier: MIT

module;

#include <base/util/force_inline.h>

export module consumer.force_inline_model;

export namespace consumer
{
    NOM_FORCE_INLINE unsigned increment(unsigned value) noexcept
    {
        return value + 1u;
    }

    struct inline_value
    {
        unsigned value;

        NOM_FORCE_INLINE unsigned get() const noexcept
        {
            return value;
        }
    };

    template <typename Value> NOM_FORCE_INLINE constexpr Value identity(Value value) noexcept
    {
        return value;
    }
} // namespace consumer
