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
    set(args EXCLUDE)
    cmake_parse_arguments(PARSE_ARGV 0 "" "" "" "${args}")

    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.12)
        set(configure_depends CONFIGURE_DEPENDS)
    endif()

    file(GLOB_RECURSE python_sources
        ${configure_depends} LIST_DIRECTORIES FALSE "${glob}"
    )
    if(_EXCLUDE)
        foreach(source ${python_sources})
            set(add TRUE)
            foreach(exclude ${_EXCLUDE})
                if(source MATCHES ${exclude})
                    set(add FALSE)
                    break()
                endif()
            endforeach()
            if(add)
                list(APPEND python_sources_filtered ${source})
            endif()
        endforeach()
    else()
        set(python_sources_filtered ${python_sources})
    endif()

    foreach(source ${python_sources_filtered})
        get_filename_component(source_name ${source} NAME_WE)

        add_test(NAME pycodestyle.${source_name}
            COMMAND ${PYCODESTYLE_EXECUTABLE} ${source}
        )
    endforeach()
endfunction()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PyCodeStyle
    REQUIRED_VARS PYCODESTYLE_EXECUTABLE
    VERSION_VAR PYCODESTYLE_VERSION_STRING
)
