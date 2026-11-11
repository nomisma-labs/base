# base

<!-- repo-description:begin -->
**base** provides common definitions for Nomisma's core libraries, including assembly,
configuration and logging. It includes format-independent configuration types, enum and
forced-inlining macros, and an owning HTTPS/WSS URL utility. Applications can construct and
validate configuration values directly in C++, without a configuration file or serialization library.
<!-- repo-description:end -->

C++ builds and consumers use CMake 4.4.x, Ninja, GCC 16 or Clang 22, and libstdc++ 16
standard-library module sources. Enable std module detection before the consumer's first
`project()` and call `nom_enable_std_modules(target)` on each compiled consumer target.
See [standard-library module setup](docs/kit/build.md#modules), including package consumers
without the kit. Macro headers remain textual includes before imports.

## Configuration types

Import `nom.config_types` and link `nom::config_types`. The public names remain in `nom`, with
convenience aliases in `nom::cfg_types`. They include numeric wrappers, strings, containers,
optional and defaulted values, and type-eligibility traits.

```cpp
import std;

import nom.config_types;

struct connection_config
{
    nom::cfg_string host{"localhost"};
    nom::cfg_uint port{9000};
    nom::cfg_defaulted<nom::cfg_double> timeout_s{2.5};
};
```

An installed consumer uses:

```cmake
find_package(base CONFIG REQUIRED)
target_link_libraries(app PRIVATE nom::config_types)
nom_enable_std_modules(app)
```

The component retains its `config_types` module and target names after moving from
`json_config`; the repository and package are named `base`. There is no `nom.base` umbrella
module. [json_config](https://github.com/nomisma-labs/json_config) supplies JSON mapping and
checked conversions separately.

The configuration-types target depends only on the standard library. Catch2 is needed only
when building tests. See the [type guide](docs/base/config_types.md),
[design note](docs/base/config_types_design.md), and
[source and installed consumer tests](tests/integration/library_usage/README.md).

## Enum and URL utilities

Include `<base/util/enum.h>` and link `nom::base_enum` to define scoped enums with explicit underlying
types, constexpr string names, and stream output using `NOM_ENUM`. `NOM_ENUM_TO_STRING` adds
names and stream output to an existing enum. Import `nom.url` and link `nom::base_url` for
secure connection endpoints:

```cpp
#include <base/util/enum.h>

import nom.url;

namespace example
{
    NOM_ENUM(status, int, disconnected, connecting, connected, failed)
}

const nom::url endpoint("wss://feed.example:9443/trades?symbol=BTC%2FUSD#local");
// endpoint.host() == "feed.example", endpoint.port() == "9443"
// endpoint.target() == "/trades?symbol=BTC%2FUSD"
static_assert(enum_to_string(example::status::connected) == "connected");
```

The URL utility owns its strings and has no individual component mutators. It preserves
encoded path/query bytes and excludes fragments from HTTP request targets. The base build
and package require Boost.URL 1.90 or newer; only `nom::base_url` links it. Config types and
enum macros remain independent at the target level.

See [utilities and legacy comparison](docs/base/utilities.md) for the API contract, deliberate
changes from the legacy helpers, limitations, examples, and validation instructions.

## Forced inlining

Include `<base/util/force_inline.h>` and link `nom::base_force_inline`:

```cpp
#include <base/util/force_inline.h>

NOM_FORCE_INLINE unsigned next_sequence(unsigned value) noexcept
{
    return value + 1u;
}
```

`NOM_FORCE_INLINE` expands to `[[gnu::always_inline]] inline` on the supported GCC/Clang
toolchains. It requires attribute support instead of silently falling back to ordinary `inline`.
For module consumers, include the header in the global module fragment and keep the annotated
definition in the module interface. See the [inlining contract](docs/base/utilities.md#forced-inlining)
for usage, compiler limitations and verification.

Configuration sources live in `src/base/config/`; utility headers and `url.cppm` live in
`src/base/util/`. Module imports and existing CMake target names remain unchanged.

## Build

Install the tools listed in [docs/kit/build.md](docs/kit/build.md#toolchain), then:

```bash
./bootstrap.sh                   # fresh builds, tests and checks for every preset but release
./bootstrap.sh complete          # one complete build, including source and installed consumers
cmake --workflow --preset dev    # configure, build and test one preset
```

Everyday commands, the presets and what bootstrap checks: [docs/kit/build.md](docs/kit/build.md).

## Docs

Agents start at [AGENTS.md](AGENTS.md), people at [docs/](docs/README.md). Adding a library, an
app, tests or a dependency: [docs/kit/howto.md](docs/kit/howto.md). Releases: [CHANGELOG.md](CHANGELOG.md).

## License

This project is licensed under [MIT](LICENSE). Copyright belongs to Alexey Tkachenko.
The synchronized build kit retains its [MIT notice](cmake/kit/LICENSE); third-party
dependencies retain their own licenses.
