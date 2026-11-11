// SPDX-FileCopyrightText: 2026 Alexey Tkachenko
// SPDX-License-Identifier: MIT

#include <base/util/force_inline.h>

import consumer.force_inline_model;

// The macro also works in an ordinary translation unit.
NOM_FORCE_INLINE unsigned decrement(unsigned value) noexcept
{
    return value - 1u;
}

static_assert(consumer::identity(42) == 42);

int main(int argc, char**)
{
    // Runtime input prevents these uses from being only constant-evaluation tests.
    const auto input = static_cast<unsigned>(argc);
    const consumer::inline_value wrapped{input};
    if (decrement(consumer::increment(input)) != input)
    {
        return 1;
    }
    if (wrapped.get() != input || consumer::identity(input) != input)
    {
        return 2;
    }
    return 0;
}
