# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

if(NOT CMAKE_COMPILER_IS_GNUC AND NOT CMAKE_COMPILER_IS_GNUCXX)
    # FAIL
endif()

include(GnuCompilerConfiguration)

if(NOT "${GNUC_BINDIR}" STREQUAL "")
    set(_gnuc_bin "${GNUC_BINDIR}")
elseif(NOT "${GNUC_PREFIX}" STREQUAL "")
    set(_gnuc_bin "${GNUC_PREFIX}/bin")
endif()

find_program(GCOV_EXECUTABLE
    NAMES
        "${GNUC_PROGRAM_PREFIX}gcov${GNUC_PROGRAM_SUFFIX}"
        "${GNUC_PROGRAM_PREFIX}gcov"
        "gcov${GNUC_PROGRAM_SUFFIX}"
        "gcov"
    PATHS "${_gnuc_bin}"
)

if(NOT "${GNUC_LIBDIR}" STREQUAL "")
    set(_gnuc_lib "${GNUC_LIBDIR}")
elseif(NOT "${GNUC_PREFIX}" STREQUAL "")
    set(_gnuc_lib "${GNUC_PREFIX}/lib/gcc/8")
endif()

find_library(GCOV_LIBRARY
    NAMES
        "${GNUC_PROGRAM_PREFIX}gcov${GNUC_PROGRAM_SUFFIX}"
        "${GNUC_PROGRAM_PREFIX}gcov"
        "gcov${GNUC_PROGRAM_SUFFIX}"
        "gcov"
    PATHS "${_gnuc_lib}/gcc/${GNUC_BUILD}/${CMAKE_CXX_COMPILER_VERSION}"
)

find_program(LCOV_EXECUTABLE NAMES lcov lcov.perl)
find_program(GENHTML_EXECUTABLE NAMES genhtml genhtml.perl)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GnuCoverage
    REQUIRED_VARS GCOV_EXECUTABLE GCOV_LIBRARY LCOV_EXECUTABLE GENHTML_EXECUTABLE
)
mark_as_advanced(GCOV_EXECUTABLE GCOV_LIBRARY LCOV_EXECUTABLE GENHTML_EXECUTABLE)

unset(_gnuc_lib)
unset(_gnuc_bin)
