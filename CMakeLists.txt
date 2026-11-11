# SPDX-FileCopyrightText: 2026 Alexey Tkachenko
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 4.4)

include(cmake/kit/std_modules.cmake)

# The version lives only here; releases are tagged v<version> (docs/kit/build.md#versioning).
project(base
    VERSION 0.6.0
    DESCRIPTION "Common definitions for Nomisma core libraries"
    LANGUAGES CXX)

include(cmake/kit/nom_kit.cmake)
nom_project_options()

if(nom_install)
    include(GNUInstallDirs)
    install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
        DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/${PROJECT_NAME})
endif()

# URL parsing is isolated in nom::base_url; the common types and enum macros do not link it.
find_package(Boost 1.90 CONFIG REQUIRED COMPONENTS url)

# Discover Catch2 before any code directory can register an inline test suite.
if(nom_build_tests)
    enable_testing()
    nom_find_catch2()
endif()

add_subdirectory(src)

if(nom_build_apps)
    add_subdirectory(apps)
endif()

if(nom_build_tests)
    add_subdirectory(tests)
endif()
