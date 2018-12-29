# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

find_program(PYCODESTYLE_EXECUTABLE NAMES pycodestyle pep8)

function(pycodestyle_add_tests glob)
    set(args EXCLUDE)
    cmake_parse_arguments(PARSE_ARGV 0 "" "" "" "${args}")

    file(GLOB_RECURSE python_sources LIST_DIRECTORIES FALSE "${glob}")
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
)
