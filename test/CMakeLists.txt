include(CTest)
include(GoogleTest)

find_package(GoogleTest 1.8.0)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(test_radix_trie
    ${CMAKE_CURRENT_SOURCE_DIR}/moat/datastruct/radix_trie.cc
)
target_link_libraries(test_radix_trie ${GTEST_BOTH_LIBRARIES})
gtest_discover_tests(test_radix_trie)
