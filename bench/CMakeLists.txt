find_package(GoogleBenchmark 1.4.1)
include_directories(${BENCHMARK_INCLUDE_DIRS})

add_executable(bench_trie ${CMAKE_CURRENT_SOURCE_DIR}/moat/trie.cc)
target_link_libraries(bench_trie ${BENCHMARK_BOTH_LIBRARIES})
