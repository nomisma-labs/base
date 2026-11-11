# SPDX-FileCopyrightText: 2026 Alexey Tkachenko
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 4.4)

include(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/kit/std_modules.cmake)
project(base_library_usage LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Consumers must work without serialization libraries or the provider's test framework.
# Boost.URL is a package dependency for the URL component, not a config_types link dependency.
set(CMAKE_DISABLE_FIND_PACKAGE_nlohmann_json TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_Catch2 TRUE)

if(LIBRARY_USAGE_MODE STREQUAL "package")
    find_package(base CONFIG REQUIRED)
else()
    include(FetchContent)
    FetchContent_Declare(base SOURCE_DIR ${LIBRARY_USAGE_SOURCE_DIR})
    FetchContent_MakeAvailable(base)
endif()

enable_testing()
add_executable(use_library use_library_test.cxx)
nom_enable_std_modules(use_library)
target_link_libraries(use_library PRIVATE nom::config_types)
add_test(NAME use_library COMMAND use_library)

get_target_property(config_dependencies nom::config_types INTERFACE_LINK_LIBRARIES)
if(config_dependencies)
    message(FATAL_ERROR "config_types must remain dependency-free: ${config_dependencies}")
endif()

# A consumer module expands the macros and exports its enum and generated overloads.
add_library(enum_model STATIC)
nom_enable_std_modules(enum_model)
target_sources(enum_model PUBLIC FILE_SET CXX_MODULES FILES enum_model.cppm)
target_link_libraries(enum_model PUBLIC nom::base_enum)

add_executable(use_utilities use_utilities_test.cxx)
nom_enable_std_modules(use_utilities)
target_link_libraries(use_utilities PRIVATE enum_model nom::base_url)
add_test(NAME use_utilities COMMAND use_utilities)

# Forced-inline definitions must remain usable across a named-module boundary.
add_library(force_inline_model STATIC)
nom_enable_std_modules(force_inline_model)
target_sources(force_inline_model PUBLIC FILE_SET CXX_MODULES FILES force_inline_model.cppm)
target_link_libraries(force_inline_model PUBLIC nom::base_force_inline)

add_executable(use_force_inline use_force_inline_test.cxx)
nom_enable_std_modules(use_force_inline)
target_link_libraries(use_force_inline PRIVATE force_inline_model)
add_test(NAME use_force_inline COMMAND use_force_inline)
