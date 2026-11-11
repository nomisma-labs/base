// SPDX-FileCopyrightText: 2026 Alexey Tkachenko
// SPDX-License-Identifier: MIT

module;

#include <boost/url/authority_view.hpp>
#include <boost/url/ipv6_address.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/url.hpp>

export module nom.url;

import std;

export namespace nom
{
    class url
    {
    public:
        explicit url(std::string_view text);
        // URI host spelling (IPv6 may be unbracketed), numeric port, encoded origin-form target.
        url(std::string_view scheme, std::string_view host, std::uint16_t port, std::string_view target);

        const std::string& scheme() const noexcept
        {
            return scheme_;
        }

        // Decoded resolver host; IPv6 addresses have no surrounding brackets.
        const std::string& host() const noexcept
        {
            return host_;
        }

        // TCP port in 1..65535; an absent or empty URI port defaults to 443.
        std::uint16_t port() const noexcept
        {
            return port_;
        }

        // HTTP origin-form: encoded path and optional query, never a fragment.
        const std::string& target() const noexcept
        {
            return target_;
        }

    private:
        void validate_host() const;

        std::string scheme_;
        std::string host_;
        std::uint16_t port_;
        std::string target_;
    };

    std::ostream& operator<<(std::ostream& output, const url& value);
} // namespace nom

namespace nom
{
    std::string normalize_scheme(std::string_view scheme)
    {
        switch (boost::urls::string_to_scheme(scheme))
        {
        case boost::urls::scheme::https:
            return "https";
        case boost::urls::scheme::wss:
            return "wss";
        default:
            throw std::invalid_argument("URL scheme must be https or wss");
        }
    }

    url::url(std::string_view text)
    {
        const auto parsed = boost::urls::parse_uri(text);
        if (!parsed)
        {
            throw std::invalid_argument("Invalid URL syntax: " + parsed.error().message());
        }

        const auto& value = *parsed;
        scheme_ = normalize_scheme(value.scheme());

        if (!value.has_authority() || value.encoded_host().empty())
        {
            throw std::invalid_argument("URL requires a nonempty host");
        }
        if (value.has_userinfo())
        {
            throw std::invalid_argument("URL user information is not supported");
        }
        if (value.host_type() == boost::urls::host_type::ipvfuture)
        {
            throw std::invalid_argument("URL IPvFuture hosts are not supported");
        }

        host_ = value.host_address();
        validate_host();

        const auto port_text = value.port();
        if (port_text.empty())
        {
            port_ = 443;
        }
        else
        {
            std::uint32_t port_number{};
            const auto [end, error] =
                std::from_chars(port_text.data(), port_text.data() + port_text.size(), port_number);
            if (error != std::errc{} || end != port_text.data() + port_text.size() || port_number == 0 ||
                port_number > 65535)
            {
                throw std::invalid_argument("URL port must be a decimal number from 1 to 65535");
            }
            port_ = static_cast<std::uint16_t>(port_number);
        }

        const auto path = value.encoded_path();
        target_ = path.empty() ? "/" : std::string(path);
        if (value.has_query())
        {
            target_ += '?';
            target_ += value.encoded_query();
        }
    }

    url::url(std::string_view scheme, std::string_view host, std::uint16_t port, std::string_view target)
        : scheme_(normalize_scheme(scheme)), port_(port), target_(target.empty() ? "/" : target)
    {
        if (boost::urls::parse_ipv6_address(host))
        {
            host_ = host;
        }
        else
        {
            const auto parsed_host = boost::urls::parse_authority(host);
            if (!parsed_host || parsed_host->has_userinfo() || parsed_host->has_port())
            {
                throw std::invalid_argument("URL host must contain only a host, without user information or a port");
            }
            if (parsed_host->host_type() == boost::urls::host_type::ipvfuture)
            {
                throw std::invalid_argument("URL IPvFuture hosts are not supported");
            }
            host_ = parsed_host->host_address();
        }
        validate_host();
        if (port_ == 0)
        {
            throw std::invalid_argument("URL port must be from 1 to 65535");
        }
        if (!boost::urls::parse_origin_form(target_))
        {
            throw std::invalid_argument("URL target must be an encoded absolute path with an optional query");
        }
    }

    void url::validate_host() const
    {
        if (host_.empty())
        {
            throw std::invalid_argument("URL requires a nonempty host");
        }
        if (std::ranges::any_of(host_, [](unsigned char ch) { return ch <= 0x20 || ch == 0x7f; }))
        {
            throw std::invalid_argument("URL host must not contain whitespace or control characters");
        }
    }

    std::ostream& operator<<(std::ostream& output, const url& value)
    {
        boost::urls::url authority;
        authority.set_host_address(value.host());
        const auto encoded_host = authority.encoded_host();
        return output << value.scheme() << "://" << std::string_view(encoded_host.data(), encoded_host.size()) << ':'
                      << std::to_string(value.port()) << value.target();
    }
} // namespace nom
