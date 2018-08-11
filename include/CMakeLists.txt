# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

add_library(Armor INTERFACE)
target_include_directories(Armor INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/ArmorConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/ArmorConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/ArmorConfig.cmake"
    INSTALL_DESTINATION lib/cmake/Armor
)
install(TARGETS Armor EXPORT ArmorTargets)
install(EXPORT ArmorTargets DESTINATION "lib/cmake/Armor")
install(FILES
    "${PROJECT_BINARY_DIR}/ArmorConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/ArmorConfig.cmake"
    DESTINATION "lib/cmake/Armor"
)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/rmr" DESTINATION "include")
