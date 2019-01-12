# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

find_library(GCOV_LIBRARY NAMES gcov)
find_library(GCOV_EXECUTABLE NAMES gcov)
find_program(LCOV_EXECUTABLE NAMES lcov lcov.bat lcov.exe lcov.perl)
find_program(GENHTML_EXECUTABLE NAMES genhtml genhtml.perl genhtml.bat)
if(NOT GCOV_LIBRARY OR NOT GCOV_EXECUTABLE OR NOT LCOV_EXECUTABLE OR NOT GENHTML_EXECUTABLE)
    message(FATAL_ERROR "Coverage tools not found, aborting.")
endif()

#------------------------------------------------------------------------------
function(_coverage_report_clone_target old new)
    get_target_property(sources ${old} SOURCES)
    get_target_property(interface_sources ${old} INTERFACE_SOURCES)
    get_target_property(compile_options ${old} COMPILE_OPTIONS)
    get_target_property(interface_compile_options ${old} INTERFACE_COMPILE_OPTIONS)
    get_target_property(compile_definitions ${old} COMPILE_DEFINITIONS)
    get_target_property(interface_compile_definitions ${old} INTERFACE_COMPILE_DEFINITIONS)
    get_target_property(include_directories ${old} INCLUDE_DIRECTORIES)
    get_target_property(interface_include_directories ${old} INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(link_libraries ${old} LINK_LIBRARIES)
    get_target_property(interface_link_libraries ${old} INTERFACE_LINK_LIBRARIES)
    get_target_property(type ${old} TYPE)

    if(type STREQUAL "STATIC_LIBRARY")
        add_library(${new} STATIC ${sources})
    elseif(type STREQUAL "MODULE_LIBRARY")
        add_library(${new} MODULE ${sources})
    elseif(type STREQUAL "SHARED_LIBRARY")
        add_library(${new} SHARED ${sources})
    elseif(type STREQUAL "OBJECT_LIBRARY")
        add_library(${new} OBJECT ${sources})
    elseif(type STREQUAL "INTERFACE_LIBRARY")
        add_library(${new} INTERFACE ${sources})
    elseif(type STREQUAL "EXECUTABLE")
        add_executable(${new} ${sources})
    else()
        message(FATAL_ERROR "Don't know how to handle targets of type '${type}'")
    endif()

    if(compile_options)
        target_compile_options(${new} PRIVATE ${compile_options})
    endif()
    if(interface_compile_options)
        target_compile_options(${new} PUBLIC ${interface_compile_options})
    endif()
    if(compile_definitions)
        target_compile_definitions(${new} PRIVATE ${compile_definitions})
    endif()
    if(interface_compile_definitions)
        target_compile_definitions(${new} PUBLIC ${interface_compile_definitions})
    endif()
    if(include_directories)
        target_include_directories(${new} PRIVATE ${include_directories})
    endif()
    if(interface_include_directories)
        target_include_directories(${new} PUBLIC ${interface_include_directories})
    endif()
    if(link_libraries)
        target_link_libraries(${new} PRIVATE ${link_libraries})
    endif()
    if(interface_link_libraries)
        target_link_libraries(${new} PUBLIC ${interface_link_libraries})
    endif()
    set_target_properties(${new} PROPERTIES INTERFACE_SOURCES ${interface_sources})
endfunction()

#------------------------------------------------------------------------------
# TODO add support for CROSSCOMPILING_EMULATOR
function(coverage_report_add)
    cmake_parse_arguments(PARSE_ARGV 0 "" "" "TARGET" "ARGUMENTS")
    if(NOT _TARGET)
        message(FATAL_ERROR "coverage_report_add() requires a target")
    endif()

    _coverage_report_clone_target(${_TARGET} ${_TARGET}_coverage)
    set_target_properties(${_TARGET}_coverage PROPERTIES EXCLUDE_FROM_ALL TRUE)
    target_compile_options(${_TARGET}_coverage PRIVATE --coverage)
    target_link_libraries(${_TARGET}_coverage PRIVATE ${GCOV_LIBRARY})

    set(run_command_silent ${CMAKE_CURRENT_BINARY_DIR}/run_command_silent.cmake)
    file(WRITE ${run_command_silent}
        "execute_process(COMMAND \"\${COMMAND}\"\n"
        "       \${ARGUMENTS}\n"
        "   WORKING_DIRECTORY \"\${WORKING_DIRECTORY}\"\n"
        "   OUTPUT_QUIET ERROR_QUIET\n"
        ")\n"
    )
    add_custom_command(TARGET ${_TARGET}_coverage POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -D "COMMAND=$<TARGET_FILE:${_TARGET}_coverage>"
            -D "ARGUMENTS=${_ARGUMENTS}"
            -D "WORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}"
            -P ${run_command_silent}
        VERBATIM
    )
    set_property(DIRECTORY APPEND PROPERTY COVERAGE_REPORT_TARGETS ${_TARGET})
endfunction()

#------------------------------------------------------------------------------
function(coverage_report_generate)
    if(NOT CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_COMPILER_IS_GNUCXX)
        message(FATAL_ERROR
            "Coverage report is only supported on GNU compilers."
        )
    endif()
    if(NOT CMAKE_BUILD_TYPE STREQUAL Debug)
        message(WARNING
            "Not building in debug mode, "
            "coverage report may not reflect actual coverage."
        )
    endif()

    set(options DEFAULT_LCOV_EXCLUDES)
    set(single_value_args TARGET OUTPUT_NAME)
    set(multi_value_args ADD_TEST LCOV_ARGUMENTS LCOV_EXCLUDES GENHTML_ARGUMENTS)
    cmake_parse_arguments(PARSE_ARGV 0
        "" "${options}" "${single_value_args}" "${multi_value_args}"
    )
    if(NOT _TARGET)
        message(FATAL_ERROR "coverage_report_generate() requires a target")
    endif()
    if(NOT _OUTPUT_NAME)
        message(FATAL_ERROR "coverage_report_generate() requires an output name")
    endif()

    if(_ADD_TEST)
        set(single_value_args NAME PERCENTAGE)
        cmake_parse_arguments("_ADD_TEST" "" "${single_value_args}" "" ${_ADD_TEST})
        if(NOT _ADD_TEST_NAME)
            set(_ADD_TEST_NAME ${_OUTPUT_NAME})
        endif()
        if(NOT _ADD_TEST_PERCENTAGE)
            set(_ADD_TEST_PERCENTAGE 95)
        endif()
    endif()

    if(_LCOV_EXCLUDES)
        list(JOIN _LCOV_EXCLUDES " " _LCOV_EXCLUDES)
    endif()
    if(_DEFAULT_LCOV_EXCLUDES)
        execute_process(
            COMMAND ${CMAKE_CXX_COMPILER} -E -v -xc++ -
            INPUT_FILE /dev/null
            OUTPUT_QUIET ERROR_VARIABLE output
        )
        string(REGEX MATCH "#include <...> search starts here:.*End of search list" output "${output}")
        string(REPLACE "\n" ";" output "${output}")
        list(REMOVE_AT output 0 -1)
        foreach(line ${output})
            get_filename_component(line "${line}" REALPATH)
            string(APPEND _LCOV_EXCLUDES " ${line}/*")
        endforeach()

        string(APPEND _LCOV_EXCLUDES " ${PROJECT_BINARY_DIR}/*")
    endif()

    set(coverage_report_script ${CMAKE_CURRENT_BINARY_DIR}/coverage_report.cmake)
    set(coverage_report_target ${_TARGET})
    set(coverage_report_base_dir "${CMAKE_CURRENT_BINARY_DIR}/CoverageReport/")
    set(coverage_report_output_dir "${coverage_report_base_dir}/${_OUTPUT_NAME}")
    set(coverage_report_log_dir "${coverage_report_base_dir}/_Logs")

    get_property(coverage_report_targets DIRECTORY PROPERTY COVERAGE_REPORT_TARGETS)
    file(WRITE ${coverage_report_script}
        "file(MAKE_DIRECTORY ${coverage_report_log_dir})\n"
        "set(coverage_report_targets\n"
    )
    foreach(target ${coverage_report_targets})
        file(APPEND ${coverage_report_script} "    \"${target}\"\n")
    endforeach()
    file(APPEND ${coverage_report_script}
        ")\n"
        "\n"
        "foreach(target \${coverage_report_targets})\n"
        "    message(STATUS \"Generating coverage information for \${target}\")\n"
        "    execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}\n"
        "               --target \${target}_coverage\n"
        "        OUTPUT_FILE \"${coverage_report_log_dir}/build_\${target}.out\"\n"
        "        ERROR_FILE \"${coverage_report_log_dir}/build_\${target}.err\"\n"
        "        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}\n"
        "    )\n"
        "endforeach()\n"
        "\n"
        "message(STATUS \"Rendering coverage information in human readable format (LCOV)\")\n"
        "execute_process(\n"
        "    COMMAND ${LCOV_EXECUTABLE} ${_LCOV_ARGUMENTS}\n"
        "           --gcov-tool \"${GCOV_EXECUTABLE}\"\n"
        "           --directory \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
        "           --capture\n"
        "           --output-file \"${coverage_report_log_dir}/lcov.first.info\"\n"
        "    OUTPUT_FILE \"${coverage_report_log_dir}/lcov.first.out\"\n"
        "    ERROR_FILE \"${coverage_report_log_dir}/lcov.first.err\"\n"
        "    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}\n"
        ")\n"
        "\n"
        "execute_process(\n"
        "    COMMAND ${LCOV_EXECUTABLE} ${_LCOV_ARGUMENTS}\n"
        "           --gcov-tool \"${GCOV_EXECUTABLE}\"\n"
        "           --remove \"${coverage_report_log_dir}/lcov.first.info\"\n"
        "               ${_LCOV_EXCLUDES}\n"
        "           --output-file \"${coverage_report_log_dir}/lcov.clean.info\"\n"
        "    OUTPUT_FILE \"${coverage_report_log_dir}/lcov.clean.out\"\n"
        "    ERROR_FILE \"${coverage_report_log_dir}/lcov.clean.err\"\n"
        "    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}\n"
        ")\n"
        "\n"
        "execute_process(\n"
        "    COMMAND ${GENHTML_EXECUTABLE} ${_GENHTML_ARGUMNETS}\n"
        "           -o \"${coverage_report_output_dir}\"\n"
        "           \"${coverage_report_log_dir}/lcov.clean.info\"\n"
        "    OUTPUT_FILE \"${coverage_report_log_dir}/genhtml.out\"\n"
        "    ERROR_FILE \"${coverage_report_log_dir}/genhtml.err\"\n"
        "    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}\n"
        ")\n"
        "\n"
        "file(RELATIVE_PATH result_dir ${PROJECT_BINARY_DIR} \"${coverage_report_output_dir}\")\n"
        "message(STATUS \"Done. Coverage information can be found in ./\${result_dir}\")\n"
    )
    add_custom_target(${coverage_report_target}
        COMMAND ${CMAKE_COMMAND} -P ${coverage_report_script}
        COMMENT "Generating coverage report"
        BYPRODUCTS "${coverage_report_base_dir}"
        VERBATIM
    )

    if(_ADD_TEST)
        set(coverage_report_file_base         "${CMAKE_CURRENT_BINARY_DIR}/${_OUTPUT_NAME}_coverage")
        set(coverage_report_write_test_file   "${coverage_report_file_base}_write_test.cmake")
        set(coverage_report_test_file         "${coverage_report_file_base}_tests.cmake")
        set(coverage_report_test_include_file "${coverage_report_file_base}_include.cmake")
        set(coverage_report_check_script      "${coverage_report_file_base}_check.cmake")
        set(coverage_report_test_name         "coverage-report/${_ADD_TEST_NAME}")

        file(WRITE ${coverage_report_check_script}
            "file(READ \${FILE} contents)\n"
            "string(REGEX MATCHALL \"<td class=\\\"headerCovTableEntryHi\\\">([0-9.]*) %</td>\" lines \"\${contents}\")\n"
            "\n"
            "set(fail FALSE)\n"
            "foreach(line \${lines})\n"
            "    string(REGEX REPLACE \"<td class=\\\"headerCovTableEntryHi\\\">\" \"\" line \"\${line}\")\n"
            "    string(REGEX REPLACE \" %</td>\" \"\" line \"\${line}\")\n"
            "    if(line LESS \${PERCENTAGE})\n"
            "        message(WARNING \"\${line} % is less than \${PERCENTAGE}\")\n"
            "        set(fail TRUE)\n"
            "    endif()\n"
            "endforeach()\n"
            "\n"
            "if(fail)\n"
            "    message(FATAL_ERROR \"Test failed because target of \${TARGET} % missed\")\n"
            "endif()\n"
        )

        file(WRITE "${coverage_report_write_test_file}"
            "file(WRITE \"${coverage_report_test_file}\"\n"
            "    \"add_test(${coverage_report_test_name}\\n\"\n"
            "    \"    ${CMAKE_COMMAND}\\n\"\n"
            "    \"        -D \\\"FILE=${coverage_report_output_dir}/index.html\\\"\\n\"\n"
            "    \"        -D \\\"PERCENTAGE=${_ADD_TEST_PERCENTAGE}\\\"\\n\"\n"
            "    \"        -P \\\"${coverage_report_check_script}\\\"\\n\"\n"
            "    \")\\n\"\n"
            ")\n"
        )
        add_custom_command(TARGET ${coverage_report_target} POST_BUILD
            BYPRODUCTS "${coverage_report_test_file}"
            COMMAND "${CMAKE_COMMAND}" -P "${coverage_report_write_test_file}"
            VERBATIM
        )

        file(WRITE "${coverage_report_test_include_file}"
            "if(EXISTS \"${coverage_report_test_file}\")\n"
            "   include(\"${coverage_report_test_file}\")\n"
            "else()\n"
            "   add_test(${coverage_report_test_name}_NOT_BUILT\n"
            "       ${coverage_report_test_name}_NOT_BUILT\n"
            "   )\n"
            "endif()\n"
        )
        set_property(DIRECTORY
            APPEND PROPERTY TEST_INCLUDE_FILES "${coverage_report_test_include_file}"
        )
    endif()
endfunction()
