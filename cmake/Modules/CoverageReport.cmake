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

function(clone_target target new_target)
    get_target_property(sources ${target} SOURCES)
    get_target_property(interface_sources ${target} INTERFACE_SOURCES)
    get_target_property(compile_options ${target} COMPILE_OPTIONS)
    get_target_property(interface_compile_options ${target} INTERFACE_COMPILE_OPTIONS)
    get_target_property(compile_definitions ${target} COMPILE_DEFINITIONS)
    get_target_property(interface_compile_definitions ${target} INTERFACE_COMPILE_DEFINITIONS)
    get_target_property(include_directories ${target} INCLUDE_DIRECTORIES)
    get_target_property(interface_include_directories ${target} INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(link_libraries ${target} LINK_LIBRARIES)
    get_target_property(interface_link_libraries ${target} INTERFACE_LINK_LIBRARIES)
    get_target_property(type ${target} TYPE)

    if(type STREQUAL "STATIC_LIBRARY")
        add_library(${new_target} STATIC ${sources})
    elseif(type STREQUAL "MODULE_LIBRARY")
        add_library(${new_target} MODULE ${sources})
    elseif(type STREQUAL "SHARED_LIBRARY")
        add_library(${new_target} SHARED ${sources})
    elseif(type STREQUAL "OBJECT_LIBRARY")
        add_library(${new_target} OBJECT ${sources})
    elseif(type STREQUAL "INTERFACE_LIBRARY")
        add_library(${new_target} INTERFACE ${sources})
    elseif(type STREQUAL "EXECUTABLE")
        add_executable(${new_target} ${sources})
    else()
        message(FATAL_ERROR
            "Don't know how to handle targets of type '${TYPE}'"
        )
    endif()

    if(compile_options)
        target_compile_options(${new_target} PRIVATE ${compile_options})
    endif()

    if(interface_compile_options)
        target_compile_options(${new_target} PUBLIC ${interface_compile_options})
    endif()

    if(compile_definitions)
        target_compile_definitions(${new_target} PRIVATE ${compile_definitions})
    endif()

    if(interface_compile_definitions)
        target_compile_definitions(${new_target} PUBLIC ${interface_compile_definitions})
    endif()

    if(include_directories)
        target_include_directories(${new_target} PRIVATE ${include_directories})
    endif()

    if(interface_include_directories)
        target_include_directories(${new_target} PUBLIC ${interface_include_directories})
    endif()

    if(link_libraries)
        target_link_libraries(${new_target} PRIVATE ${link_libraries})
    endif()

    if(interface_link_libraries)
        target_link_libraries(${new_target} PUBLIC ${interface_link_libraries})
    endif()

    set_target_properties(${new_target}
        PROPERTIES INTERFACE_SOURCES ${interface_sources}
    )
endfunction()

function(report_coverage target)
    set(args ARGUMENTS DEPENDENCIES LCOV_ARGS GENHTML_ARGS)
    cmake_parse_arguments("" "" "" "${args}" ${ARGN})

    clone_target(${target} ${target}_coverage)
    set_target_properties(${target}_coverage PROPERTIES EXCLUDE_FROM_ALL TRUE)
    target_compile_options(${target}_coverage PRIVATE --coverage)
    target_link_libraries(${target}_coverage PRIVATE ${GCOV_LIBRARY})

    set(run_command_silent ${CMAKE_CURRENT_BINARY_DIR}/_run_command_silent.cmake)
    file(WRITE ${run_command_silent}
        "execute_process(COMMAND \"\${COMMAND}\"\n"
        "   WORKING_DIRECTORY \"\${WORKING_DIRECTORY}\"\n"
        "   OUTPUT_QUIET ERROR_QUIET\n"
        ")\n"
    )
    add_custom_command(TARGET ${target}_coverage POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -D "COMMAND=$<TARGET_FILE:${target}_coverage>"
            -D "WORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}"
            -P ${run_command_silent}
        VERBATIM
    )
    set_property(GLOBAL APPEND PROPERTY COVERAGE_TARGETS ${target}_coverage)
endfunction()

function(generate_coverage_report report_target)
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

    set(args LCOV_ARGUMENTS LCOV_EXCLUDES GENHTML_ARGUMENTS)
    cmake_parse_arguments(PARSE_ARGV 1 "" "DEFAULT_LCOV_EXCLUDES" "" "${args}")
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

    set(coverage_report ${CMAKE_CURRENT_BINARY_DIR}/_CoverageReport.cmake)
    set(COVERAGE_REPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}/CoverageReport)
    get_property(coverage_targets GLOBAL PROPERTY COVERAGE_TARGETS)
    file(MAKE_DIRECTORY ${COVERAGE_REPORT_DIR})
    file(WRITE ${coverage_report}
        "set(coverage_targets\n"
    )
    foreach(target ${coverage_targets})
        file(APPEND ${coverage_report} "    \"${target}\"\n")
    endforeach()
    file(APPEND ${coverage_report}
        ")\n"
        "\n"
        "foreach(target \${coverage_targets})\n"
        "    execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}\n"
        "               --target \${target}\n"
        "        OUTPUT_FILE \"${COVERAGE_REPORT_DIR}/build_\${target}.out\"\n"
        "        ERROR_FILE \"${COVERAGE_REPORT_DIR}/build_\${target}.err\"\n"
        "        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}\n"
        "    )\n"
        "endforeach()\n"
        "execute_process(\n"
        "    COMMAND ${LCOV_EXECUTABLE} ${_LCOV_ARGUMENTS}\n"
        "           --gcov-tool \"${GCOV_EXECUTABLE}\"\n"
        "           --directory \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
        "           --capture\n"
        "           --output-file \"${COVERAGE_REPORT_DIR}/lcov.first.info\"\n"
        "    OUTPUT_FILE \"${COVERAGE_REPORT_DIR}/lcov.first.out\"\n"
        "    ERROR_FILE \"${COVERAGE_REPORT_DIR}/lcov.first.err\"\n"
        "    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}\n"
        ")\n"
        "execute_process(\n"
        "    COMMAND ${LCOV_EXECUTABLE} ${_LCOV_ARGUMENTS}\n"
        "           --gcov-tool \"${GCOV_EXECUTABLE}\"\n"
        "           --remove \"${COVERAGE_REPORT_DIR}/lcov.first.info\"\n"
        "               ${_LCOV_EXCLUDES}\n"
        "           --output-file \"${COVERAGE_REPORT_DIR}/lcov.clean.info\"\n"
        "    OUTPUT_FILE \"${COVERAGE_REPORT_DIR}/lcov.clean.out\"\n"
        "    ERROR_FILE \"${COVERAGE_REPORT_DIR}/lcov.clean.err\"\n"
        "    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}\n"
        ")\n"
        "execute_process(\n"
        "    COMMAND ${GENHTML_EXECUTABLE} ${_GENHTML_ARGUMNETS}\n"
        "           -o ${CMAKE_CURRENT_BINARY_DIR}/${report_target}\n"
        "           \"${COVERAGE_REPORT_DIR}/lcov.clean.info\"\n"
        "    OUTPUT_FILE \"${COVERAGE_REPORT_DIR}/genhtml.out\"\n"
        "    ERROR_FILE \"${COVERAGE_REPORT_DIR}/genhtml.err\"\n"
        "    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}\n"
        ")\n"
    )

    add_custom_target(${report_target}
        COMMAND ${CMAKE_COMMAND} -P ${coverage_report}
        COMMENT "Generating coverage report (this may take a while)"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${coverage_report}"
    )
endfunction()
