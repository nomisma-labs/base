# Library consumer tests

This standalone project constructs connection settings, checks C++ defaults, modifies values,
stores them in nested containers, and runs application validation. It imports
`nom.config_types` and links `nom::config_types`, with no configuration format or file.
The consumer checks that `nom::config_types` has no link dependencies. nlohmann/json and
Catch2 package discovery are disabled; Boost.URL is required by the package's URL component.

A second executable imports `nom.url` and a consumer-defined module which expands the enum
macros. It checks IPv6 host extraction, encoded request targets without fragments, enum
stringification/streaming through the module boundary, and invalid-port rejection.

A third executable imports a consumer module with `NOM_FORCE_INLINE` free functions, a class
member and a constexpr template, and also expands the macro in an ordinary translation unit.
It exercises the new `<base/util/force_inline.h>` header and `nom::base_force_inline` target.
These compile/runtime checks validate usage across module and package boundaries; they do
not replace inspecting generated code at performance-critical call sites.

`nom_library_usage_tests()` builds and runs the same consumer through `FetchContent` from
source and through `find_package(base)` from an installation. The installed-package checks
also verify that consumers can rebuild the exported module interface with their own compiler.

The consumer enables standard-library module detection before `project()` and opts each
compiled target into `CXX_MODULE_STD` through `nom_enable_std_modules()`. Its sources import
`std` directly when using standard-library declarations; macro headers remain textual includes.

From the repository root:

```sh
./bootstrap.sh complete
ctest --test-dir build/complete -R 'base_library_usage_' --output-on-failure
```

The source check remains available with `nom_install=OFF`; the install fixture and package
consumer require installation to be enabled.
