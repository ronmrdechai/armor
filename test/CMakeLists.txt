# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.10)
    include(GoogleTest)
    function(armor_add_tests target)
        gtest_discover_tests(${target})
    endfunction()
else()
    message(WARNING
        "CMake version is less than 3.10. "
        "Armor tests will not be run individually"
    )
    function(armor_add_tests target)
        add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
    endfunction()
endif()

find_package(ExternalGoogleTest 1.8.0)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.12)
    set(configure_depends CONFIGURE_DEPENDS)
endif()
file(GLOB test_sources ${configure_depends} "rmr/*.cc")

if(Armor_COVERAGE_REPORT)
    include(CoverageReport)
endif()

foreach(source ${test_sources})
    get_filename_component(source_name ${source} NAME_WE)
    set(target "test_${source_name}")

    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE Armor ${GTEST_BOTH_LIBRARIES})
    target_include_directories(${target} PRIVATE ${GTEST_INCLUDE_DIRS})
    if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
        target_compile_options(${target} PRIVATE -Wno-gnu-zero-variadic-macro-arguments)
    endif()

    armor_add_tests(${target})
    if(Armor_COVERAGE_REPORT)
        coverage_report_add(TARGET ${target})
    endif()
endforeach()

if(Armor_COVERAGE_REPORT)
    coverage_report_generate(TARGET coverage-report OUTPUT_NAME "Armor"
        ADD_TEST
            NAME "armor" PERCENTAGE 95
        DEFAULT_LCOV_EXCLUDES
    )
endif()
