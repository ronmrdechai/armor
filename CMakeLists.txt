cmake_minimum_required(VERSION 3.11)

# Configure version.
if(NOT DEFINED Moat_VERSION_MAJOR)
    set(Moat_VERSION_MAJOR 0)
endif()
if(NOT DEFINED Moat_VERSION_MINOR)
    set(Moat_VERSION_MINOR 1)
endif()
if(NOT DEFINED Moat_VERSION_PATCH)
    set(Moat_VERSION_PATCH 0)
endif()
if (NOT PACKAGE_VERSION)
    set(PACKAGE_VERSION
        "${Moat_VERSION_MAJOR}.${Moat_VERSION_MINOR}.${Moat_VERSION_PATCH}"
    )
endif()

project(Moat
    VERSION "${Moat_VERSION_MAJOR}.${Moat_VERSION_MINOR}.${Moat_VERSION_PATCH}"
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
set(CMAKE_CXX_FLAGS             "-Wall -Werror -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG       "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE     "-O3 -funroll-loops -march=native")

# Configure CPack.
set(CPACK_PACKAGE_INSTALL_DIRECTORY "moat")
set(CPACK_PACKAGE_VENDOR "moat")
set(CPACK_PACKAGE_VERSION_MAJOR ${Moat_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${Moat_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${Moat_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.TXT")
include(CPack)

include_directories(src)
add_subdirectory(src)

option(MOAT_INCLUDE_TESTS "Generate build targets for the Moat unit tests." ON)
enable_testing()
if(MOAT_INCLUDE_TESTS)
    add_subdirectory(test)
endif()

option(MOAT_INCLUDE_BENCHMARKS "Generate build targets for the Moat benchmarks." ON)
if(MOAT_INCLUDE_BENCHMARKS)
    add_subdirectory(bench)
endif()
