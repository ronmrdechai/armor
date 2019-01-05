# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.4)

# Configure version.
if(NOT DEFINED Armor_VERSION_MAJOR)
    set(Armor_VERSION_MAJOR 0)
endif()
if(NOT DEFINED Armor_VERSION_MINOR)
    set(Armor_VERSION_MINOR 1)
endif()
if(NOT DEFINED Armor_VERSION_PATCH)
    set(Armor_VERSION_PATCH 0)
endif()
if (NOT PACKAGE_VERSION)
    set(PACKAGE_VERSION
        "${Armor_VERSION_MAJOR}.${Armor_VERSION_MINOR}.${Armor_VERSION_PATCH}"
    )
endif()

project(Armor
    VERSION "${Armor_VERSION_MAJOR}.${Armor_VERSION_MINOR}.${Armor_VERSION_PATCH}"
    LANGUAGES CXX
)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Configure C++.
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS        YES)
set(CXX_FLAGS_LIST              -Wall -Werror -Wextra -pedantic)
foreach(FLAG ${CXX_FLAGS_LIST})
    if(NOT "${CMAKE_CXX_FLAGS}" MATCHES "${FLAG}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
    endif()
endforeach()
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -march=native")

# Configure CPack.
set(CPACK_PACKAGE_INSTALL_DIRECTORY "armor")
set(CPACK_PACKAGE_VENDOR "armor")
set(CPACK_PACKAGE_VERSION_MAJOR ${Armor_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${Armor_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${Armor_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
include(CPack)

option(ARMOR_INCLUDE_TESTS "Generate build targets for Armor unit tests." OFF)
option(ARMOR_COVERAGE_REPORT "Generate build targets for Armor coverage report." OFF)
option(ARMOR_LINT_TESTS "Generate linting tests." OFF)
option(ARMOR_INCLUDE_BENCHMARKS "Generate build targets for Armor benchmarks." OFF)

if(ARMOR_COVERAGE_REPORT AND CMAKE_COMPILER_IS_GNUCXX AND CMAKE_BUILD_TYPE STREQUAL Debug)
    set(ARMOR_COVERAGE_REPORT TRUE)
endif()

enable_testing()
if(ARMOR_LINT_TESTS AND ARMOR_INCLUDE_TESTS)
    include(LicenseHeaderCheck)

    license_header_add_tests("Boost Software License"
        EXCLUDE .gitignore README.md
    )
endif()

add_subdirectory(include)
add_subdirectory(debug)

if(ARMOR_INCLUDE_TESTS)
    add_subdirectory(test)
endif()

if(ARMOR_INCLUDE_BENCHMARKS)
    add_subdirectory(bench)
endif()
