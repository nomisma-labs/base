# Changelog

Notable changes to this repo. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and versions follow [Semantic Versioning](https://semver.org); how to release:
[docs/kit/build.md](docs/kit/build.md#versioning).

## [Unreleased]

## [0.6.0] - 2026-11-12

### Changed

- Publish the current library from a new root commit and retire earlier release tags.
  Consumers must update their pinned base commit; public APIs and runtime behavior are unchanged.
- Consolidate the current configuration types, enum and forced-inlining macros, and URL module
  in this release, with the existing source and installed-package integration checks.
