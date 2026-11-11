// SPDX-FileCopyrightText: 2026 Alexey Tkachenko
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import std;

import nom.url;

static_assert(!std::is_default_constructible_v<nom::url>);
static_assert(!std::is_convertible_v<std::string_view, nom::url>);
static_assert(std::is_same_v<decltype(std::declval<const nom::url&>().port()), std::uint16_t>);

TEST_CASE("Secure endpoints default to port 443 and a root path", "[url]")
{
    const nom::url endpoint("wss://feed.example");
    CHECK(endpoint.scheme() == "wss");
    CHECK(endpoint.host() == "feed.example");
    CHECK(endpoint.port() == 443);
    CHECK(endpoint.target() == "/");
    CHECK(nom::url("https://feed.example:").port() == 443);
    CHECK(nom::url("WSS://feed.example").scheme() == "wss");
    CHECK(nom::url("HtTpS://feed.example").scheme() == "https");
}

TEST_CASE("Request targets retain escaped octets and exclude fragments", "[url]")
{
    const nom::url endpoint("https://feed.example:8443/a%2Fb/%20?q=a%26b&token=%23value#local");
    CHECK(endpoint.host() == "feed.example");
    CHECK(endpoint.port() == 8443);
    CHECK(endpoint.target() == "/a%2Fb/%20?q=a%26b&token=%23value");
    CHECK(nom::url("wss://feed.example?channel=trades").target() == "/?channel=trades");
    CHECK(nom::url("wss://feed.example?").target() == "/?");
    CHECK(nom::url("wss://feed.example/path?#fragment").target() == "/path?");
    CHECK(nom::url("wss://feed.example#fragment").target() == "/");
    CHECK(nom::url("wss://feed.example/a/../b?x=1&x=2").target() == "/a/../b?x=1&x=2");
}

TEST_CASE("Resolver hosts are decoded and IPv6 brackets are removed", "[url]")
{
    const nom::url endpoint("wss://[2001:db8::1]:9443/ws");
    CHECK(endpoint.host() == "2001:db8::1");
    CHECK(endpoint.port() == 9443);
    CHECK(endpoint.target() == "/ws");
    CHECK(nom::url("https://[::1]").host() == "::1");
    CHECK(nom::url("https://127.0.0.1").host() == "127.0.0.1");
    CHECK(nom::url("https://feed%2Eexample").host() == "feed.example");
    CHECK(nom::url("https://localhost").host() == "localhost");
}

TEST_CASE("URL values own their components independently of the input and copies", "[url]")
{
    std::string text = "wss://feed.example:9443/path?channel=trades";
    const nom::url endpoint(text);
    text.assign(text.size(), 'x');
    auto copy = endpoint;
    copy = nom::url("https://other.example/new");
    CHECK(endpoint.host() == "feed.example");
    CHECK(endpoint.port() == 9443);
    CHECK(endpoint.target() == "/path?channel=trades");
    CHECK(copy.host() == "other.example");
    CHECK(copy.target() == "/new");
}

TEST_CASE("Ports are numeric and validate TCP port boundaries", "[url]")
{
    CHECK(nom::url("https://feed.example:1").port() == 1);
    CHECK(nom::url("https://feed.example:65535").port() == 65535);
    CHECK(nom::url("https://feed.example:00443").port() == 443);
}

TEST_CASE("Separate URL components use the same endpoint validation", "[url]")
{
    const nom::url endpoint("WsS", "feed%2Eexample", 9443, "/a%2Fb?symbol=BTC%2FUSD&x=%23");
    const nom::url parsed("WsS://feed%2Eexample:9443/a%2Fb?symbol=BTC%2FUSD&x=%23");
    CHECK(endpoint.scheme() == parsed.scheme());
    CHECK(endpoint.host() == parsed.host());
    CHECK(endpoint.port() == parsed.port());
    CHECK(endpoint.target() == parsed.target());

    CHECK(nom::url("https", "localhost", 443, "").target() == "/");
    CHECK(nom::url("https", "127.0.0.1", 1, "/?").port() == 1);
    CHECK(nom::url("wss", "::1", 65535, "/?").host() == "::1");
    CHECK(nom::url("wss", "[2001:db8::1]", 443, "/?").host() == "2001:db8::1");
    CHECK(nom::url("wss", "feed.example", 443, "/?").target() == "/?");
    CHECK(nom::url("wss", "feed.example", 443, "//path/a/../b?x=1&x=2").target() == "//path/a/../b?x=1&x=2");
}

TEST_CASE("Separately supplied URL strings are owned", "[url]")
{
    std::string scheme = "https", host = "feed.example", target = "/feed?channel=trades";
    const nom::url endpoint(scheme, host, 8443, target);
    scheme = "wss";
    host = "other.example";
    target = "/new";
    CHECK(endpoint.scheme() == "https");
    CHECK(endpoint.host() == "feed.example");
    CHECK(endpoint.port() == 8443);
    CHECK(endpoint.target() == "/feed?channel=trades");
}

TEST_CASE("Separate URL components cannot introduce other components", "[url]")
{
    for (const auto scheme : {"", "http", "ws", "ftp", "wss://other.example/", "wss:"})
    {
        CAPTURE(scheme);
        CHECK_THROWS_AS(nom::url(scheme, "feed.example", 443, "/"), std::invalid_argument);
    }
    for (const auto host : {"",
                            "user@feed.example",
                            "feed.example:443",
                            "feed.example:",
                            "feed.example/path",
                            "feed.example?query",
                            "feed.example#fragment",
                            "feed%00.example",
                            "feed%20.example",
                            "feed%0A.example",
                            "[v1.example]",
                            "[::1",
                            "[::1]:443",
                            "2001:db8::invalid",
                            "feed%GG.example"})
    {
        CAPTURE(host);
        CHECK_THROWS_AS(nom::url("wss", host, 443, "/"), std::invalid_argument);
    }
    for (const auto target : {"relative",
                              "?query",
                              "https://other.example/",
                              "/#fragment",
                              "/path with spaces",
                              "/%",
                              "/%GG",
                              "/?q=%",
                              "/\r\nHeader: injected"})
    {
        CAPTURE(target);
        CHECK_THROWS_AS(nom::url("wss", "feed.example", 443, target), std::invalid_argument);
    }
    CHECK_THROWS_AS(nom::url("wss", "feed.example", 0, "/"), std::invalid_argument);
    CHECK_THROWS_AS(nom::url(std::string_view("wss\0", 4), "feed.example", 443, "/"), std::invalid_argument);
    CHECK_THROWS_AS(nom::url("wss", std::string_view("feed\0.example", 13), 443, "/"), std::invalid_argument);
    CHECK_THROWS_AS(nom::url("wss", "feed.example", 443, std::string_view("/feed\0", 6)), std::invalid_argument);
}

TEST_CASE("Malformed and unsupported endpoint URLs fail before connection", "[url]")
{
    constexpr std::string_view invalid[] = {"",
                                            "feed.example",
                                            "//feed.example/ws",
                                            "https:feed.example",
                                            "https:///path",
                                            "https://",
                                            "wss://:443/path",
                                            "http://feed.example",
                                            "ws://feed.example",
                                            "ftp://feed.example",
                                            "wss://user@feed.example",
                                            "wss://user:secret@feed.example",
                                            "wss://@feed.example",
                                            "wss://feed.example:0",
                                            "wss://feed.example:65536",
                                            "wss://feed.example:999999999999999999999",
                                            "wss://feed.example:abc",
                                            "wss://feed.example:-1",
                                            "wss://feed.example:+443",
                                            "wss://feed.example/path with spaces",
                                            "wss://feed.example/%",
                                            "wss://feed.example/%GG",
                                            "wss://feed.example/?q=%",
                                            "wss://feed.example/#%GG",
                                            "wss://[::1",
                                            "wss://::1/",
                                            "wss://[v1.example]/",
                                            "wss://feed%00.example",
                                            "wss://feed%20.example",
                                            "wss://feed%0A.example"};
    for (const auto text : invalid)
    {
        CAPTURE(text);
        CHECK_THROWS_AS(nom::url(text), std::invalid_argument);
    }

    std::string with_nul = "wss://feed.example";
    with_nul += '\0';
    with_nul += "/path";
    CHECK_THROWS_AS(nom::url(with_nul), std::invalid_argument);
}

TEST_CASE("URLs print as normalized endpoints with encoded hosts and targets", "[url]")
{
    struct print_case
    {
        std::string_view input;
        std::string_view expected;
    };
    constexpr print_case cases[] = {
        {"WSS://feed.example", "wss://feed.example:443/"},
        {"https://127.0.0.1:00443/path?", "https://127.0.0.1:443/path?"},
        {"wss://[2001:db8::1]:9443/a%2Fb?symbol=BTC%2FUSD#local", "wss://[2001:db8::1]:9443/a%2Fb?symbol=BTC%2FUSD"},
        {"https://feed%2Fexample:8443/", "https://feed%2Fexample:8443/"},
        {"https://feed%25example/?x=%23", "https://feed%25example:443/?x=%23"},
    };
    for (const auto& item : cases)
    {
        CAPTURE(item.input);
        const nom::url endpoint(item.input);
        std::ostringstream output;
        output << std::hex;
        CHECK(&(output << endpoint) == &output);
        CHECK(output.str() == item.expected);
        const nom::url round_trip(output.str());
        CHECK(round_trip.scheme() == endpoint.scheme());
        CHECK(round_trip.host() == endpoint.host());
        CHECK(round_trip.port() == endpoint.port());
        CHECK(round_trip.target() == endpoint.target());
    }
}
