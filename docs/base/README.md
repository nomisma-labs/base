# Common definitions

`base` owns reusable definitions shared by Nomisma's core libraries.

- [Configuration types](config_types.md): storage types, eligibility traits, construction and
  CMake integration.
- [Configuration types and serialization dependencies](config_types_design.md): the boundary
  between common data and format-specific adapters.
- [Utilities](utilities.md): enum macros, forced inlining, URL values, usage and tests.

The `base` package exports `nom::config_types`, the macro-only `nom::base_enum` and
`nom::base_force_inline`, and `nom::base_url`. Sources live in `src/base/config/` and
`src/base/util/`; the named modules are `nom.config_types` and
`nom.url`, without an umbrella module. Boost.URL is a package/build dependency, used only
by the URL target.
