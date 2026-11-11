// SPDX-FileCopyrightText: 2026 Alexey Tkachenko
// SPDX-License-Identifier: MIT

#pragma once

#include <ostream>
#include <string_view>

// Adapted from the legacy enum utility; recursive __VA_OPT__ expansion originates at:
// https://www.scs.stanford.edu/~dm/blog/va-opt.html
#define NOM_ENUM_DETAIL_PARENS ()
#define NOM_ENUM_DETAIL_EXPAND(arg)                                                                                    \
    NOM_ENUM_DETAIL_EXPAND1(NOM_ENUM_DETAIL_EXPAND1(NOM_ENUM_DETAIL_EXPAND1(NOM_ENUM_DETAIL_EXPAND1(arg))))
#define NOM_ENUM_DETAIL_EXPAND1(arg)                                                                                   \
    NOM_ENUM_DETAIL_EXPAND2(NOM_ENUM_DETAIL_EXPAND2(NOM_ENUM_DETAIL_EXPAND2(NOM_ENUM_DETAIL_EXPAND2(arg))))
#define NOM_ENUM_DETAIL_EXPAND2(arg)                                                                                   \
    NOM_ENUM_DETAIL_EXPAND3(NOM_ENUM_DETAIL_EXPAND3(NOM_ENUM_DETAIL_EXPAND3(NOM_ENUM_DETAIL_EXPAND3(arg))))
#define NOM_ENUM_DETAIL_EXPAND3(arg)                                                                                   \
    NOM_ENUM_DETAIL_EXPAND4(NOM_ENUM_DETAIL_EXPAND4(NOM_ENUM_DETAIL_EXPAND4(NOM_ENUM_DETAIL_EXPAND4(arg))))
#define NOM_ENUM_DETAIL_EXPAND4(arg) arg

#define NOM_ENUM_DETAIL_FOR_EACH(macro, type_name, ...)                                                                \
    __VA_OPT__(NOM_ENUM_DETAIL_EXPAND(NOM_ENUM_DETAIL_FOR_EACH_HELPER(macro, type_name, __VA_ARGS__)))
#define NOM_ENUM_DETAIL_FOR_EACH_HELPER(macro, type_name, name, ...)                                                   \
    macro(type_name, name)                                                                                             \
        __VA_OPT__(NOM_ENUM_DETAIL_FOR_EACH_AGAIN NOM_ENUM_DETAIL_PARENS(macro, type_name, __VA_ARGS__))
#define NOM_ENUM_DETAIL_FOR_EACH_AGAIN() NOM_ENUM_DETAIL_FOR_EACH_HELPER

#define NOM_ENUM_DETAIL_CASE(type_name, name)                                                                          \
    case type_name::name:                                                                                              \
        return std::string_view{#name};

// Expand at namespace scope, beside the enum. List bare enumerator names with distinct values.
#define NOM_ENUM_TO_STRING(type_name, ...)                                                                             \
    constexpr std::string_view enum_to_string(type_name nom_enum_value) noexcept                                       \
    {                                                                                                                  \
        switch (nom_enum_value)                                                                                        \
        {                                                                                                              \
            NOM_ENUM_DETAIL_FOR_EACH(NOM_ENUM_DETAIL_CASE, type_name, __VA_ARGS__)                                     \
        default:                                                                                                       \
            return std::string_view{#type_name "::<unknown>"};                                                         \
        }                                                                                                              \
    }                                                                                                                  \
    inline std::ostream& operator<<(std::ostream& nom_enum_stream, type_name nom_enum_value)                           \
    {                                                                                                                  \
        return nom_enum_stream << enum_to_string(nom_enum_value);                                                      \
    }

// Declare a scoped enum with an explicit underlying type, plus names and stream output.
#define NOM_ENUM(type_name, base_type, ...)                                                                            \
    enum class type_name : base_type                                                                                   \
    {                                                                                                                  \
        __VA_ARGS__                                                                                                    \
    };                                                                                                                 \
    NOM_ENUM_TO_STRING(type_name, __VA_ARGS__)
