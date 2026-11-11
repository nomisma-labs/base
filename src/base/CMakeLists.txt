# SPDX-FileCopyrightText: 2026 Alexey Tkachenko
# SPDX-License-Identifier: MIT

nom_lib(config_types IMPORT_STD STATIC MODULES config/config_types.cppm)
set_target_properties(config_types PROPERTIES POSITION_INDEPENDENT_CODE ON)

nom_lib(base_enum HEADERS util/enum.h)
nom_lib(base_force_inline HEADERS util/force_inline.h)

nom_lib(base_url IMPORT_STD STATIC MODULES util/url.cppm PRIVATE_DEPS Boost::url)
set_target_properties(base_url PROPERTIES POSITION_INDEPENDENT_CODE ON)

nom_install_package(TARGETS config_types base_enum base_force_inline base_url
    DEPENDENCIES "Boost 1.90 CONFIG COMPONENTS url")
