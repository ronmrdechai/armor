# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

find_program(PYCODESTYLE_EXECUTABLE NAMES pycodestyle pep8)
if(PYCODESTYLE_EXECUTABLE)
    execute_process(COMMAND ${PYCODESTYLE_EXECUTABLE} --version
        OUTPUT_VARIABLE PYCODESTYLE_VERSION_STRING
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

function(pycodestyle_add_tests glob)
    set(args EXCLUDE RELATIVE ARGS)
    cmake_parse_arguments(PARSE_ARGV 1 "" "" "" "${args}")

    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.12)
        set(configure_depends CONFIGURE_DEPENDS)
    endif()

    if(_RELATIVE)
        set(relative RELATIVE ${_RELATIVE})
    endif()

    if(NOT _ARGS)
        set(_ARGS --show-source --show-pep8)
    endif()

    file(GLOB_RECURSE python_sources
        ${configure_depends} ${relative} LIST_DIRECTORIES FALSE "${glob}"
    )
    if(_EXCLUDE)
        list(JOIN _EXCLUDE "|" exclude_regex)
        list(FILTER python_sources EXCLUDE REGEX ${exclude_regex})
    endif()

    foreach(source ${python_sources})
        get_filename_component(source_name ${source} NAME_WE)
        get_filename_component(source_path ${source} DIRECTORY)

        set(test_name "${source_path}/${source_name}")
        string(REGEX REPLACE "^/" "" test_name ${test_name})
        string(REGEX REPLACE "/" "." test_name ${test_name})
        string(REGEX REPLACE "\.__init__$" "" test_name ${test_name})

        add_test(NAME pycodestyle/${test_name}
            COMMAND ${PYCODESTYLE_EXECUTABLE} ${_RELATIVE}/${source} ${_ARGS}
        )
    endforeach()
endfunction()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PyCodeStyle
    REQUIRED_VARS PYCODESTYLE_EXECUTABLE
    VERSION_VAR PYCODESTYLE_VERSION_STRING
)
mark_as_advanced(PYCODESTYLE_EXECUTABLE PYCODESTYLE_VERSION_STRING)
