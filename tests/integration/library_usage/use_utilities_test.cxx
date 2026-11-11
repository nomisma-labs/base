// SPDX-FileCopyrightText: 2026 Alexey Tkachenko
// SPDX-License-Identifier: MIT

// Ubuntu GCC 16's snapshot ICEs in fortified snprintf for this importing stream consumer
// at -O2. Making the stdio declarations visible before import std avoids the compiler crash.
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 16
#include <cstdio>
#endif

import std;

import consumer.enum_model;
import nom.url;

static_assert(enum_to_string(consumer::status::connected) == "connected");
static_assert(std::is_same_v<decltype(std::declval<const nom::url&>().port()), std::uint16_t>);

int main()
{
    const nom::url endpoint("wss://[::1]:9443/feed%2Ftrades?symbol=BTC%2FUSD#local");
    if (endpoint.scheme() != "wss" || endpoint.host() != "::1" || endpoint.port() != 9443 ||
        endpoint.target() != "/feed%2Ftrades?symbol=BTC%2FUSD")
    {
        return 1;
    }

    const nom::url components("WSS", "::1", 9443, "/feed%2Ftrades?symbol=BTC%2FUSD");
    if (components.scheme() != endpoint.scheme() || components.host() != endpoint.host() ||
        components.port() != endpoint.port() || components.target() != endpoint.target())
    {
        return 4;
    }

    std::ostringstream output;
    output << components;
    if (output.str() != "wss://[::1]:9443/feed%2Ftrades?symbol=BTC%2FUSD")
    {
        return 5;
    }
    output.str("");
    output << consumer::status::connected << ' ' << static_cast<consumer::status>(255);
    if (output.str() != "connected status::<unknown>")
    {
        return 2;
    }

    try
    {
        const nom::url invalid("wss://host.example:65536");
        return 3;
    }
    catch (const std::invalid_argument&)
    {
        return 0;
    }
}
