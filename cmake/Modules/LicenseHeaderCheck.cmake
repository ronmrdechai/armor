# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

find_package(Git)
if(NOT GIT_FOUND)
    function(license_header_add_tests)
    endfunction()
    return()
endif()


function(license_header_add_tests license)
    cmake_parse_arguments(PARSE_ARGV 1 "" "" "" "EXCLUDE")

    set(search_text_script "${CMAKE_CURRENT_BINARY_DIR}/search_text.cmake")
    file(WRITE ${search_text_script}
        "file(READ \"\${SEARCH_FILE}\" contents)\n"
        "if(NOT contents MATCHES \"\${SEARCH_TERM}\")\n"
        "    message(FATAL_ERROR \"Text '\${SEARCH_TERM}' not found in '\${SEARCH_FILE}'\")\n"
        "endif()\n"
    )

    execute_process(COMMAND ${GIT_EXECUTABLE} ls-files
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE git_files
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    separate_arguments(git_files NATIVE_COMMAND "${git_files}")
    if(_EXCLUDE)
        list(JOIN _EXCLUDE "|" exclude_regex)
        list(FILTER git_files EXCLUDE REGEX ${exclude_regex})
    endif()

    foreach(file ${git_files})
        add_test(NAME license/${file}
            COMMAND ${CMAKE_COMMAND}
                -D "SEARCH_FILE=${CMAKE_CURRENT_SOURCE_DIR}/${file}"
                -D "SEARCH_TERM=${license}"
                -P ${search_text_script}
        )
    endforeach()
endfunction()
