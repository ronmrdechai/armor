# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

find_package(GoogleBenchmark 1.4.1)
include_directories(${BENCHMARK_INCLUDE_DIRS})

add_executable(bench_trie_map ${CMAKE_CURRENT_SOURCE_DIR}/rmr/trie_map.cc)
target_link_libraries(bench_trie_map Armor ${BENCHMARK_BOTH_LIBRARIES})
