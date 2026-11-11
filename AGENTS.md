# AGENTS.md

<!-- repo-description:begin -->
`base` owns common definitions for Nomisma's core libraries: format-independent
configuration types, enum declaration/stringification and forced-inlining macros, and an owning
HTTPS/WSS URL module. Config types and macros have no third-party dependencies; URL parsing uses
Boost.URL internally. Catch2 is used only for tests. Start with
[docs/base/README.md](docs/base/README.md).
<!-- repo-description:end -->

Read and follow the rules every Nomisma repo shares: @docs/kit/agents.md

Rules for this repo alone follow below.

- Sources live in `src/base/config/` and `src/base/util/`. Preserve module `nom.config_types` and target
  `nom::config_types`; these names intentionally differ from the repository name. There is
  currently no `nom.base` umbrella module. The installed CMake package is `base`.
- `<base/util/enum.h>` provides only `NOM_ENUM(name, underlying_type, ...)` and `NOM_ENUM_TO_STRING`
  through the macro-only target `nom::base_enum`. `NOM_ENUM` declares a scoped enum with an
  explicit underlying type. Keep internal helpers under `NOM_ENUM_DETAIL_`.
- `<base/util/force_inline.h>` provides `NOM_FORCE_INLINE` through `nom::base_force_inline`.
  It includes `inline` and requires `gnu::always_inline` support; keep annotated definitions
  reachable by importing callers. Use selectively and verify critical call sites in generated code.
- `nom.url` exports `nom::url` through `nom::base_url`. Keep Boost types out of its public
  signatures, preserve encoded request targets, and keep config_types and enum targets
  independent of Boost at the link/interface level.
- Keep format-specific parsers, mappings, conversion checks and dependencies in their adapter
  repositories. Common config wrappers are transparent storage; applications validate domain
  rules and adapters validate source representations.
