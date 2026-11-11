// SPDX-FileCopyrightText: 2026 Alexey Tkachenko
// SPDX-License-Identifier: MIT

import std;

import nom.config_types;

namespace consumer
{
    struct connection_config
    {
        nom::cfg_string host{"localhost"};
        nom::cfg_uint port{9000};
        nom::cfg_defaulted<nom::cfg_double> timeout_s{2.5};
        nom::cfg_optional<nom::cfg_string> interface_name;

        std::expected<void, std::string> validate() const
        {
            if (host.empty() || port == 0 || port > 65535 || *timeout_s <= 0)
            {
                return std::unexpected("connection requires a host, valid port and positive timeout");
            }
            return {};
        }
    };
} // namespace consumer

template <> inline constexpr bool nom::enable_config_type<consumer::connection_config> = true;

static_assert(nom::is_config_value<nom::cfg_vector<consumer::connection_config>>);
static_assert(nom::is_config_member<nom::cfg_optional<consumer::connection_config>>);

int main()
{
    nom::cfg_defaulted<consumer::connection_config> connection;
    if (!connection->validate() || connection->host != "localhost" || connection->port != 9000 ||
        *connection->timeout_s != 2.5 || connection->interface_name)
    {
        return 1;
    }

    consumer::connection_config& settings = connection;
    settings.host = "feed.example";
    settings.port.get() = 443;
    settings.timeout_s->get() = 1.25;
    settings.interface_name = "eth0";

    nom::cfg_map<nom::cfg_vector<consumer::connection_config>> feeds{{"primary", {*connection}}};
    const auto& primary = feeds.at("primary").front();
    if (!primary.validate() || primary.host != "feed.example" || primary.port != 443 || *primary.timeout_s != 1.25 ||
        primary.interface_name != "eth0")
    {
        return 2;
    }

    // Storage accepts C++ assignments; the application owns domain validation.
    settings.port.get() = 70000;
    if (settings.validate() || settings.port != 70000)
    {
        return 3;
    }
    return 0;
}
