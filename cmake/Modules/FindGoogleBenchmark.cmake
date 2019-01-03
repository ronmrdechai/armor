# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

include(ExternalProject)

if(NOT PACKAGE_FIND_VERSION)
    set(BENCHMARK_TAG master)
else()
    set(BENCHMARK_TAG "v${PACKAGE_FIND_VERSION}")
endif()

ExternalProject_Add(googlebenchmark
    GIT_REPOSITORY   "https://github.com/google/benchmark.git"
    GIT_TAG          ${BENCHMARK_TAG}
    GIT_SHALLOW      TRUE
    UPDATE_COMMAND   ""
    INSTALL_COMMAND  ""
    CMAKE_ARGS       -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                     -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                     -DCMAKE_BUILD_TYPE=Release
                     -DBENCHMARK_ENABLE_TESTING:BOOL=OFF
    LOG_DOWNLOAD     ON
    LOG_CONFIGURE    ON
    LOG_BUILD        ON
    BUILD_BYPRODUCTS
        <BINARY_DIR>/src/${CMAKE_FIND_LIBRARY_PREFIXES}benchmark.a
        <BINARY_DIR>/src/${CMAKE_FIND_LIBRARY_PREFIXES}benchmark_main.a
)
set(BENCHMARK_VERSION_STRING ${PACKAGE_FIND_VERSION})

ExternalProject_Get_Property(googlebenchmark source_dir)
set(BENCHMARK_INCLUDE_DIRS ${source_dir}/include)

ExternalProject_Get_Property(googlebenchmark binary_dir)
set(BENCHMARK_LIBRARY_PATH ${binary_dir}/src/${CMAKE_FIND_LIBRARY_PREFIXES}benchmark.a)
set(BENCHMARK_LIBRARY benchmark)
add_library(${BENCHMARK_LIBRARY} UNKNOWN IMPORTED)
set_target_properties(${BENCHMARK_LIBRARY} PROPERTIES
    IMPORTED_LOCATION ${BENCHMARK_LIBRARY_PATH}
)
add_dependencies(${BENCHMARK_LIBRARY} googlebenchmark)

set(BENCHMARK_MAIN_LIBRARY_PATH ${binary_dir}/src/${CMAKE_FIND_LIBRARY_PREFIXES}benchmark_main.a)
set(BENCHMARK_MAIN_LIBRARY benchmark_main)
add_library(${BENCHMARK_MAIN_LIBRARY} UNKNOWN IMPORTED)
set_target_properties(${BENCHMARK_MAIN_LIBRARY} PROPERTIES
    IMPORTED_LOCATION ${BENCHMARK_MAIN_LIBRARY_PATH}
)
add_dependencies(${BENCHMARK_MAIN_LIBRARY} googlebenchmark)
set(BENCHMARK_BOTH_LIBRARIES ${BENCHMARK_LIBRARY} ${BENCHMARK_MAIN_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GoogleBenchmark
    REQUIRED_VARS
        BENCHMARK_INCLUDE_DIRS BENCHMARK_LIBRARY_PATH BENCHMARK_LIBRARY
        BENCHMARK_MAIN_LIBRARY_PATH BENCHMARK_MAIN_LIBRARY_PATH BENCHMARK_BOTH_LIBRARIES
    VERSION_VAR BENCHMARK_VERSION_STRING
)
mark_as_advanced(
    BENCHMARK_INCLUDE_DIRS BENCHMARK_LIBRARY_PATH BENCHMARK_LIBRARY
    BENCHMARK_MAIN_LIBRARY_PATH BENCHMARK_MAIN_LIBRARY_PATH BENCHMARK_BOTH_LIBRARIES
)
