// SPDX-FileCopyrightText: 2026 Alexey Tkachenko
// SPDX-License-Identifier: MIT

module;

#include <base/util/enum.h>

export module consumer.enum_model;

import std;

export namespace consumer
{
    NOM_ENUM(status, std::uint8_t, disconnected, connecting, connected, failed)
} // namespace consumer
