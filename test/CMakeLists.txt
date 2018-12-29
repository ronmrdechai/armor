# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

include(CTest)
include(GoogleTest)

find_package(GoogleTest 1.8.0)
find_package(PyCodeStyle)

file(GLOB test_sources CONFIGURE_DEPENDS "rmr/*.cc")

foreach(source ${test_sources})
    get_filename_component(source_name ${source} NAME_WE)
    set(target "test_${source_name}")

    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE Armor ${GTEST_BOTH_LIBRARIES})
    target_include_directories(${target} PRIVATE ${GTEST_INCLUDE_DIRS})
    gtest_discover_tests(${target})
endforeach()

if(PyCodeStyle_FOUND)
    pycodestyle_add_tests("${PROJECT_SOURCE_DIR}/*.py"
        EXCLUDE __init__.py "${PROJECT_BINARY_DIR}"
    )
endif()
