// SPDX-FileCopyrightText: 2026 Alexey Tkachenko
// SPDX-License-Identifier: MIT

#pragma once

#if !__has_cpp_attribute(gnu::always_inline)
#error "NOM_FORCE_INLINE requires support for [[gnu::always_inline]] (GCC or Clang)."
#endif

// Keep the definition reachable by callers, including through a module interface.
// Includes inline: use before the return type, without an additional inline keyword.
#define NOM_FORCE_INLINE [[gnu::always_inline]] inline
