find_package(GoogleBenchmark 1.4.1)
include_directories(${BENCHMARK_INCLUDE_DIRS})

add_executable(bench_trie_map ${CMAKE_CURRENT_SOURCE_DIR}/rmr/trie_map.cc)
target_link_libraries(bench_trie_map ${BENCHMARK_BOTH_LIBRARIES})
