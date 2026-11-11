# Utilities

These utilities live in `src/base/util/` and are independent components in the `base` package.

| Component | Use | CMake target |
|---|---|---|
| Enum macros | `#include <base/util/enum.h>` | `nom::base_enum` |
| Forced inlining | `#include <base/util/force_inline.h>` | `nom::base_force_inline` |
| URL value | `import nom.url;` | `nom::base_url` |

The existing `nom.config_types` module and `nom::config_types` target are unchanged. There is
no `nom.base` umbrella module. The URL implementation uses Boost.URL 1.90 or newer; its public
API exposes only standard-library types. The build and installed `base` package discover
Boost.URL, but `config_types`, `base_enum` and `base_force_inline` do not link it. Avoiding that package
discovery for config-only consumers would require separate/component package support.

## Enum contract

Expand the macros at namespace scope beside the enum:

```cpp
#include <base/util/enum.h>
import std;

namespace example
{
    NOM_ENUM(direction, int, ask, bid)
    NOM_ENUM(status, int, disconnected, connecting, connected, failed)
    NOM_ENUM(wire_status, std::uint8_t, idle, ready)

    enum class result : unsigned { ok = 0, rejected = 400 };
    NOM_ENUM_TO_STRING(result, ok, rejected)
}

static_assert(enum_to_string(example::status::connected) == "connected");
```

Each macro supplies `constexpr std::string_view enum_to_string(Enum) noexcept` and an inline
stream insertion operator in the calling namespace. ADL finds them. Known values produce
the enumerator's spelling; unmapped values produce `type_name::<unknown>`. Returned names
refer to string literals and need no allocation.

`NOM_ENUM(name, underlying_type, ...)` always declares a scoped enum and requires its underlying
type explicitly. Only `NOM_ENUM` and `NOM_ENUM_TO_STRING` are public enum macros; no alternate
declaration forms or compatibility aliases are provided.

`NOM_ENUM` accepts bare enumerator names. For explicit numeric values, declare the
enum normally and use `NOM_ENUM_TO_STRING`; listed values must be distinct because the mapping
uses a switch. Aliases may be omitted in favor of one canonical name. There is no string-to-enum
parser, flags decomposition, or runtime reflection. Unit tests cover 64-entry expansion; this
does not promise an unlimited preprocessor list size.

To declare/export an enum inside a named module, include the header in the global module
fragment, then expand the macro in an exported namespace. The source and installed consumers
exercise this pattern.

## Forced inlining

`<base/util/force_inline.h>` defines `NOM_FORCE_INLINE` as `[[gnu::always_inline]] inline`.
Link the header-only `nom::base_force_inline` target to obtain the include path, including
from an installed package. The header has no includes or link dependencies and rejects
compilers without the attribute. GCC 16 and Clang 22 support it.

Use it before the return type on a small, measured hot-path function. The macro already
includes `inline`; it can also precede `constexpr`. In a named module:

```cpp
module;
#include <base/util/force_inline.h>

export module example.sequence;

export namespace example
{
    NOM_FORCE_INLINE unsigned next_sequence(unsigned value) noexcept
    {
        return value + 1u;
    }
}
```

Keep the full definition in the module interface so importing translation units can see it.
Consumers only need `import example.sequence;` to call this function; they include the macro
header themselves only when annotating their own definitions. A class-body definition in a
named module is not implicitly inline; annotate the member explicitly when forcing inlining.

The attribute bypasses ordinary inlining cost heuristics. It does not make unavailable
definitions visible, resolve arbitrary indirect calls, or make recursive/coroutine calls
universally inlineable. GCC diagnoses failure to inline a supported direct call; Clang
explicitly does not guarantee substitution in every situation. See the
[GCC attribute documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Attributes.html#always_inline)
and [Clang attribute documentation](https://clang.llvm.org/docs/AttributeReference.html#always-inline-force-inline).

Check critical importing call sites using compiler optimization reports and generated assembly
with the production flags. GCC provides `-fopt-info-inline-optimized-missed`; Clang provides
`-Rpass=inline -Rpass-missed=inline -Rpass-analysis=inline`. An ordinary runtime test alone
does not demonstrate inlining. Source and installed consumers compile free functions, class
members and a constexpr template across a module boundary; code-generation inspection is a
separate validation step. Prefer ordinary `inline` for small helpers unless measurements
justify forcing it, since indiscriminate inlining can increase code size and instruction-cache pressure.

## URL contract

```cpp
import std;
import nom.url;

const nom::url endpoint("wss://[::1]:9443/a%2Fb?symbol=BTC%2FUSD#local");
// scheme(): "wss"
// host():   "::1"
// port():   9443 (std::uint16_t)
// target(): "/a%2Fb?symbol=BTC%2FUSD"
```

`explicit url(std::string_view)` accepts an absolute HTTPS or WSS URI with a nonempty authority
host. It owns its components independently of the input. `scheme()`, `host()`, and `target()`
return `const std::string&`; `port()` returns `std::uint16_t` by value. All getters are `noexcept`. The object has no default constructor or individual
component mutators. References follow normal string/value lifetime rules, including after a
move or whole-object assignment.

An absent or empty URI port defaults to `443`. An explicit URI port must be decimal, in
1..65535; leading zeroes are accepted and the value is stored numerically. An empty path
becomes `/`. A present query retains its `?` even when empty. Fragments are accepted but never
included in the request target.

`host()` is decoded for resolver use and strips IPv6 brackets. User information, IPvFuture
hosts, and decoded host whitespace/control characters are rejected instead of silently
passing unsupported connection settings onward. This is a syntactic connection-endpoint
utility: it does not resolve DNS, convert international hostnames through IDNA, construct
HTTP Host headers, configure TLS/SNI, or validate certificates.

The four-component constructor is:

```cpp
const nom::url endpoint("wss", "::1", 9443, "/feed?symbol=BTC%2FUSD");
std::cout << endpoint; // wss://[::1]:9443/feed?symbol=BTC%2FUSD
```

`url(std::string_view scheme, std::string_view host, std::uint16_t port, std::string_view target)`
accepts the same case-insensitive HTTPS/WSS schemes. The host uses URI spelling, including
percent escapes in registered names; IPv6 may be bracketed or unbracketed. It cannot include
user information or a port. The numeric port must be nonzero. The target is an encoded
absolute path with an optional query; an empty target becomes `/`. Relative paths, fragments,
spaces, control characters, and malformed escapes are rejected. Component strings are copied.

`operator<<` prints `scheme://host:port` followed by the encoded target. It always includes
the numeric port in decimal, even when the stream uses hexadecimal formatting. IPv6 hosts
get brackets and reserved characters in decoded host names are percent-escaped. A parsed
URI's original scheme case, port padding, and fragment are not reproduced. Streaming takes
O(n) time and O(h) temporary storage for a host of length h.

Malformed/unsupported input throws `std::invalid_argument`; allocation failures retain
their normal exception type. Construction takes O(n) time and storage in the input length;
getters are O(1). Ordinary copies own independent strings.

## Validation

```sh
./bootstrap.sh complete
ctest --preset complete -R 'base_(enum_tests|url_tests|library_usage_)'
tools/kit/checks/naming.sh
```

Unit tests cover scoped enums with explicit underlying types, sparse numeric values, unknown
values, macro name collisions, stream output, long enumerator lists, owned URL storage,
escaped request targets, fragments, empty queries, IPv4/IPv6, case-insensitive schemes,
numeric ports, component-boundary validation, stream output, malformed input, and decoded host controls. Integration consumers verify both source
and installed-package usage, including macro-generated declarations and forced-inline functions
crossing a module boundary, plus ordinary translation-unit use of `NOM_FORCE_INLINE`.

The optimized importing stream consumer triggers an internal compiler error in `snprintf`
with Ubuntu GCC 16's 20260322 snapshot (`nonnull_arg_p`, `tree.cc:14919`). Including `<cstdio>`
before `import std;` avoids the reproduced crash; the integration fixture retains that header
for GCC 16. Optimization and `_FORTIFY_SOURCE` remain enabled. Clang does not need this workaround.

Enum and URL utilities were introduced in `base` v0.2.1. The `util/` include layout and
`NOM_FORCE_INLINE` are available starting with v0.2.2; older versions use `<base/enum.h>`.

The numeric URL port, four-component constructor, and stream operator are available in v0.3.0.
