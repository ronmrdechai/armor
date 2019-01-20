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
if(NOT PACKAGE_VERSION)
    set(PACKAGE_VERSION
        "${Armor_VERSION_MAJOR}.${Armor_VERSION_MINOR}.${Armor_VERSION_PATCH}"
    )
endif()

project(Armor
    VERSION "${Armor_VERSION_MAJOR}.${Armor_VERSION_MINOR}.${Armor_VERSION_PATCH}"
    LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Configure C++.
include(CheckCXXCompilerFlag)
macro(armor_append_flag testname flag ${ARGN})
    check_cxx_compiler_flag("${flag} ${ARGN}" ${testname})
    if(${testname})
        add_compile_options(${flag} ${ARGN})
    endif()
endmacro()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CXX_FLAGS_LIST -Wall -Werror -Wextra -pedantic)
foreach(FLAG ${CXX_FLAGS_LIST})
    if(NOT "${CMAKE_CXX_FLAGS}" MATCHES "${FLAG}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
    endif()
endforeach()
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -march=native")

# Options and subdirectories.
option(Armor_ADDRESS_SANITIZER "Run tests using AddressSanitizer." OFF)
if(Armor_ADDRESS_SANITIZER)
    cmake_policy(SET CMP0056 NEW)
    set(asan_flags "-fsanitize=address,undefined")
    if(NOT APPLE)
        string(APPEND asan_flags ",integer,nullability,leak")
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES Clang AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.0)
        set(asan_flags "-fsanitize=address")
    endif()

    set(cmake_exe_linker_flags_save "${CMAKE_EXE_LINKER_FLAGS}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " ${asan_flags}")

    armor_append_flag(ARMOR_HAS_ASAN "${asan_flags}")
    if(NOT ARMOR_HAS_ASAN)
        set(CMAKE_EXE_LINKER_FLAGS ${cmake_exe_linker_flags_save})
    endif()
    if(APPLE AND ARMOR_HAS_ASAN)
        add_compile_options(-fvisibility=hidden)
    endif()
endif()

option(Armor_RUN_CLANG_TIDY "Run clang-tidy with the compiler." OFF)
if(Armor_RUN_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
    if(NOT CLANG_TIDY_EXECUTABLE)
        message(FATAL_ERROR "Armor_RUN_CLANG_TIDY is ON but clang-tidy is not found.")
    endif()
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}")
    configure_file(.clang-tidy .clang-tidy COPYONLY)
endif()

option(Armor_RUN_IWYU "Run include-what-you-use with the compiler." OFF)
if(Armor_RUN_IWYU)
    find_program(IWYU_EXECUTABLE NAMES include-what-you-use iwyu)
    if(NOT IWYU_EXECUTABLE)
        message(FATAL_ERROR "Armor_RUN_IWYU is ON but include-what-you-use is not found!")
    endif()
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU_EXECUTABLE}"
        -Xiwyu --mapping_file=${PROJECT_SOURCE_DIR}/.iwyu.imp -w ${Armor_IWYU_OPTIONS}
    )
endif()

option(Armor_LINT_TESTS "Generate linting tests." OFF)
option(Armor_INCLUDE_TESTS "Generate build targets for Armor unit tests." OFF)
if(Armor_LINT_TESTS AND Armor_INCLUDE_TESTS)
    set(Armor_LINT_TESTS ON)
else()
    set(Armor_LINT_TESTS OFF)
endif()

enable_testing()
if(Armor_LINT_TESTS)
    include(LicenseHeaderCheck)
    license_header_add_tests("Boost Software License" EXCLUDE "^\\\." README.md)

    # TODO format code according to clang-format
    # find_package(ClangFormat)
    # if(ClangFormat_FOUND)
    #     clang_format_add_tests("include/*.h" RELATIVE "${PROJECT_SOURCE_DIR}/include")
    # endif()
endif()

add_subdirectory(include)
add_subdirectory(debug)

option(Armor_COVERAGE_REPORT "Generate build targets for Armor coverage report." OFF)
if(Armor_INCLUDE_TESTS)
    add_subdirectory(test)
endif()

option(Armor_INCLUDE_BENCHMARKS "Generate build targets for Armor benchmarks." OFF)
if(Armor_INCLUDE_BENCHMARKS)
    add_subdirectory(bench)
endif()

# Configure CPack.
set(CPACK_PACKAGE_INSTALL_DIRECTORY "armor")
set(CPACK_PACKAGE_VENDOR "armor")
set(CPACK_PACKAGE_VERSION_MAJOR ${Armor_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${Armor_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${Armor_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
include(CPack)
