// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <iostream>

#include <benchmark/benchmark.h>

#include <rmr/trie_set.h>
#include <rmr/tst_set.h>

struct alpha {
    std::size_t operator()(std::size_t c) const { return ('a' <= c && c <= 'z') ? c - 'a' : c - 'A'; }
};

struct trie_set : rmr::trie_set<26, alpha> { using rmr::trie_set<26, alpha>::trie_set; };
struct tst_set  : rmr::tst_set<>    { using rmr::tst_set<>::tst_set;      };

namespace bench {

std::vector<std::size_t> random_indices(std::size_t n, std::size_t max) {
    std::default_random_engine engine(std::random_device{}());
    std::uniform_int_distribution dist(0ul, max);

    std::vector<std::size_t> indices;
    for (std::size_t i = 0; i < n; i++) indices.push_back(dist(engine));
    std::sort(indices.begin(), indices.end());

    return indices;
}

std::vector<std::string> random_words(std::size_t n) {
    std::ifstream words_file(WORDS_FILE); 
    std::size_t word_count = std::count(
        std::istreambuf_iterator<char>(words_file), std::istreambuf_iterator<char>(), '\n'
    );

    auto indices = random_indices(n, word_count);

    words_file.clear();
    words_file.seekg(0, std::ios::beg);

    std::vector<std::string> words;
    std::size_t target_line = 0;
    for (std::size_t current_line = 0; current_line < word_count; current_line++) {
        std::string line;
        std::getline(words_file, line);
        if (current_line == indices[target_line]) {
            words.push_back(line);
            target_line++;
        }
    }

    return words;
}

} // namespace bench

#define ARMOR_TEMPLATE_BENCHMARK(func, param)\
    void func##_##param(benchmark::State& s) { func<param>(s); }\
    BENCHMARK(func##_##param)

template <typename T>
void insertion(benchmark::State& state) {
    auto words = bench::random_words(state.range(0));

    for (auto _ : state) {
        state.PauseTiming();
        T t;
        state.ResumeTiming();

        t.insert(words.begin(), words.end());
    }
}

ARMOR_TEMPLATE_BENCHMARK(insertion, trie_set)->RangeMultiplier(10)->Range(10, 100000);
ARMOR_TEMPLATE_BENCHMARK(insertion,  tst_set)->RangeMultiplier(10)->Range(10, 100000);
