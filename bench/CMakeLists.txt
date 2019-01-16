# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

find_package(ExternalGoogleBenchmark 1.4.1)
find_package(WordsFile REQUIRED)

file(GLOB test_sources CONFIGURE_DEPENDS "rmr/*.cc")

foreach(source ${test_sources})
    get_filename_component(source_name ${source} NAME_WE)
    set(target "bench_${source_name}")

    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE Armor ${BENCHMARK_BOTH_LIBRARIES})
    target_include_directories(${target} PRIVATE ${BENCHMARK_INCLUDE_DIRS})
    target_compile_definitions(${target} PRIVATE "WORDS_FILE=\"${WORDS_FILE_PATH}\"")
endforeach()
