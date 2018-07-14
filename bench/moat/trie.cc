#include <benchmark/benchmark.h>

#include <moat/trie.h>

using trie = moat::ascii_trie<int>;

static void BM_trie_insertion(benchmark::State& state) {
    trie t;
    for (auto _ : state) {
        t[std::string(state.range(0), '-')] = 1; 
    }
}
BENCHMARK(BM_trie_insertion)->RangeMultiplier(2)->Range(1, 1 << 5);
