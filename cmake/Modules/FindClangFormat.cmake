# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

find_program(CLANG_FORMAT_EXECUTABLE
    NAMES
        clang-format
        clang-format-${ClangFormat_FIND_VERSION_MAJOR}
        clang-format-${ClangFormat_FIND_VERSION}
)

if(CLANG_FORMAT_EXECUTABLE)
    execute_process(COMMAND ${CLANG_FORMAT_EXECUTABLE} -version
        OUTPUT_VARIABLE CLANG_FORMAT_VERSION_STRING
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REPLACE "clang-format version " ""
        CLANG_FORMAT_VERSION_STRING "${CLANG_FORMAT_VERSION_STRING}"
    )
    string(REGEX REPLACE " \\(.*\\)$" ""
        CLANG_FORMAT_VERSION_STRING "${CLANG_FORMAT_VERSION_STRING}"
    )
endif()

function(clang_format_add_tests glob)
    set(args EXCLUDE RELATIVE ARGS)

    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.12)
        set(configure_depends CONFIGURE_DEPENDS)
    endif()

    if(_RELATIVE)
        set(relative RELATIVE ${_RELATIVE})
    endif()

    file(GLOB_RECURSE sources
        ${configure_depends} ${relative} LIST_DIRECTORIES FALSE "${glob}"
    )
    if(_EXCLUDE)
        list(JOIN _EXCLUDE "|" exclude_regex)
        list(FILTER sources EXCLUDE REGEX ${exclude_regex})
    endif()

    set(format_check "${CMAKE_CURRENT_BINARY_DIR}/format_check.cmake")
    file(WRITE ${format_check}
        "file(READ \"\${INPUT_FILE}\" contents)\n"
        "execute_process(\n"
        "    COMMAND \"\${FORMAT_EXECUTABLE}\" \"\${FORMAT_OPTIONS}\" \"\${INPUT_FILE}\"\n"
        "    OUTPUT_VARIABLE format_contents\n"
        "    ERROR_QUIET\n"
        "    WORKING_DIRECTORY \"\${WORKING_DIRECTORY}\"\n"
        ")\n"
        "if(NOT contents STREQUAL format_contents)\n"
        "    find_program(diff_executable NAMES colordiff diff)\n"
        "    if(diff_executable)\n"
        "        execute_process(\n"
        "            COMMAND \"\${FORMAT_EXECUTABLE}\" \"\${FORMAT_OPTIONS}\" \"\${INPUT_FILE}\"\n"
        "            COMMAND \"\${diff_executable}\" -u \"\${INPUT_FILE}\"\n -"
        "            ERROR_QUIET\n"
        "            WORKING_DIRECTORY \"\${WORKING_DIRECTORY}\"\n"
        "        )\n"
        "    endif()\n"
        "    message(FATAL_ERROR \"File \${INPUT_FILE} not formatted correctly\")\n"
        "endif()\n"
    )

    foreach(source ${sources})
        if(NOT RELATIVE)
            set(relative ${PROJECT_SOURCE_DIR})
        endif()

        file(RELATIVE_PATH test_name "${relative}" "${source}")
        add_test(NAME clang-format/${test_name}
            COMMAND ${CMAKE_COMMAND}
                -D "INPUT_FILE=${_RELATIVE}/${source}"
                -D "FORMAT_EXECUTABLE=${CLANG_FORMAT_EXECUTABLE}"
                -D "FORMAT_ARGS=${_ARGS}"
                -D "WORKING_DIRECTORY=${PROJECT_SOURCE_DIR}"
                -P ${format_check}
        )
    endforeach()
endfunction()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangFormat
    REQUIRED_VARS CLANG_FORMAT_EXECUTABLE
    VERSION_VAR CLANG_FORMAT_VERSION_STRING
)
mark_as_advanced(CLANG_FORMAT_EXECUTABLE CLANG_FORMAT_VERSION_STRING)
