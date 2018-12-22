# Armor: An STL-compliant header-only C++ container library

_Armor_ is a C++ library containing various non-standard containers and
algorithms. Many different implementations of the same concepts exist, and all
are rigorously benchmarked.

## Things included in Armor
The following datastructures are currently included in _Armor_, or will be in
the near future:

### Associative containers

All of _Armor's_ associative containers are fully STL compatible and satisfy the
`AllocatorAwareContainer` concept requirements.

#### Trie-based containers

- [x] `trie_map` - An implementation of a Trie based map.
- [x] `tst_map` - An implementation of a Ternary Search Tree based map.
- [ ] `compressed_trie_map` - An implementation of a compressed Trie based map.
- [ ] `patricia_trie_map` - An implementation of a PARTICIA Trie based map.
- [ ] `hat_trie_map` - An implementation of a
[HAT Trie](http://crpit.com/confpapers/CRPITV62Askitis.pdf) based map.

#### DAWG-based containers

- [ ] `dawg_map` - An implementation of a Directed Acyclic Word Graph based map.
- [ ] `compressed_dawg_map` - An implementation of a compressed DAWG based map.

### Set containers

In addition to the `map` containers, _Armor_ provides the same containers in
`set` form. For example, if you want to use a `trie_map` as a set, just use
`trie_set` instead.

### Strings

_Armor_ provides additional datastuctures for processing of strings. Again, all
of the following types satisfy the `AllocatorAwareContainer` concept
requirements and are fully STL compatible.

- [ ] `rope` - An implementation of a Rope-based string.
- [ ] `gap_buffer` - An implementation of a Gap Buffer-base string.

## Prerequisites

A compiler supporting C++17 is required to build the tests and use the library.
_Armor_ also downloads and builds its own copy of
[Google Test](https://github.com/google/googletest) and
[Google Benchmark](https://github.com/google/benchmark) when building the tests
and benchmarks.

## Building Tests and Benchmarks

_Armor_ is built with `cmake`. To build _Armor_ tests and benchmarks create a
new directory and run `cmake`.

    mkdir build
    cd build
    cmake .. -G Ninja -DARMOR_INCLUDE_TESTS:BOOL=ON -DARMOR_INCLUDE_BENCHMARKS:BOOL=ON
    cmake --build .
    ctest
