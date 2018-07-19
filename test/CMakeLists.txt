# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

include(CTest)
include(GoogleTest)

find_package(GoogleTest 1.8.0)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(test_trie_map ${CMAKE_CURRENT_SOURCE_DIR}/rmr/trie_map.cc)
target_link_libraries(test_trie_map ${GTEST_BOTH_LIBRARIES})
gtest_discover_tests(test_trie_map)
