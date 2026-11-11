// SPDX-FileCopyrightText: 2026 Alexey Tkachenko
// SPDX-License-Identifier: MIT

// Include first: the utility must supply its own ostream and string_view dependencies.
#include <base/util/enum.h>

#include <catch2/catch_test_macros.hpp>

import std;

#if defined(EXPAND) || defined(FOR_EACH) || defined(PARENS)
#error The enum utility must not define generic preprocessor helpers
#endif

#if defined(ENUM) || defined(ENUM_CLASS) || defined(ENUM_CLASS_BASE_TYPE) || defined(ENUM_TO_STRING) ||                \
    defined(NOM_ENUM_CLASS) || defined(NOM_ENUM_CLASS_BASE_TYPE)
#error Only NOM_ENUM and NOM_ENUM_TO_STRING are supported public enum macros
#endif

namespace fixture
{
    NOM_ENUM(direction, int, ask, bid)
    NOM_ENUM(status, int, disconnected, connecting, connected, failed)
    NOM_ENUM(byte_status, std::uint8_t, value, nom_enum_value, nom_enum_stream)

    enum class error_code : std::uint16_t
    {
        ok = 0,
        invalid_request = 400,
        timeout = 408
    };
    NOM_ENUM_TO_STRING(error_code, ok, invalid_request, timeout)

    NOM_ENUM(empty_status, int)

    // Keep enough entries to exercise multiple rounds of recursive expansion.
    NOM_ENUM(many_values,
             int,
             v00,
             v01,
             v02,
             v03,
             v04,
             v05,
             v06,
             v07,
             v08,
             v09,
             v10,
             v11,
             v12,
             v13,
             v14,
             v15,
             v16,
             v17,
             v18,
             v19,
             v20,
             v21,
             v22,
             v23,
             v24,
             v25,
             v26,
             v27,
             v28,
             v29,
             v30,
             v31,
             v32,
             v33,
             v34,
             v35,
             v36,
             v37,
             v38,
             v39,
             v40,
             v41,
             v42,
             v43,
             v44,
             v45,
             v46,
             v47,
             v48,
             v49,
             v50,
             v51,
             v52,
             v53,
             v54,
             v55,
             v56,
             v57,
             v58,
             v59,
             v60,
             v61,
             v62,
             v63)
} // namespace fixture

namespace other_fixture
{
    NOM_ENUM(status, int, connected)
} // namespace other_fixture

static_assert(fixture::enum_to_string(fixture::status::connected) == "connected");
static_assert(noexcept(fixture::enum_to_string(fixture::status::connected)));
static_assert(std::is_scoped_enum_v<fixture::status>);
static_assert(std::is_same_v<std::underlying_type_t<fixture::status>, int>);
static_assert(std::is_same_v<std::underlying_type_t<fixture::byte_status>, std::uint8_t>);

TEST_CASE("Enum names are available through ADL for scoped enums", "[enum]")
{
    CHECK(enum_to_string(fixture::direction::ask) == "ask");
    CHECK(enum_to_string(fixture::direction::bid) == "bid");
    CHECK(enum_to_string(fixture::status::failed) == "failed");
    CHECK(enum_to_string(other_fixture::status::connected) == "connected");
    CHECK(enum_to_string(fixture::byte_status::value) == "value");
    CHECK(enum_to_string(fixture::byte_status::nom_enum_value) == "nom_enum_value");
    CHECK(enum_to_string(fixture::byte_status::nom_enum_stream) == "nom_enum_stream");
}

TEST_CASE("Existing enums retain explicit values and deterministic unknown names", "[enum]")
{
    CHECK(static_cast<std::uint16_t>(fixture::error_code::timeout) == 408);
    CHECK(enum_to_string(fixture::error_code::invalid_request) == "invalid_request");
    CHECK(enum_to_string(static_cast<fixture::error_code>(404)) == "error_code::<unknown>");
    CHECK(enum_to_string(static_cast<fixture::status>(-1)) == "status::<unknown>");
    CHECK(enum_to_string(static_cast<fixture::byte_status>(255)) == "byte_status::<unknown>");
    CHECK(enum_to_string(static_cast<fixture::empty_status>(0)) == "empty_status::<unknown>");
}

TEST_CASE("Stream output uses the same enum names and unknown fallback", "[enum]")
{
    std::ostringstream output;
    output << fixture::direction::bid << ' ' << fixture::status::connected << ' '
           << static_cast<fixture::byte_status>(255);
    CHECK(output.str() == "bid connected byte_status::<unknown>");
}

TEST_CASE("Enum mapping expands a substantial enumerator list", "[enum]")
{
    CHECK(enum_to_string(fixture::many_values::v00) == "v00");
    CHECK(enum_to_string(fixture::many_values::v31) == "v31");
    CHECK(enum_to_string(fixture::many_values::v63) == "v63");
}
