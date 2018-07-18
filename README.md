# Armor: An STL-compliant header-only C++ container library

_Armor_ is a C++ library containing various non-standard containers and
algorithms. Many different implementations of the same concepts exist, and all
are rigorously benchmarked.

## Things included in Armor
The following containers are currently included in _Armor_, or will be in the
near future:

- [x] `trie_map` - An implementation of a Trie based map.
- [ ] `tst_map` - An implementation of a Ternary Search Tree based map.
- [ ] `ctrie_map` - An implementation of a compressed Trie based map.
- [ ] `ptrie_map` - An implementation of a PARTICIA Trie based map.
- [ ] `htrie_map` - An implementation of a [HAT Trie](http://crpit.com/confpapers/CRPITV62Askitis.pdf) based map.
- [ ] `trie_set` - A Trie based map to set adaptor.

## Building Tests and Benchmarks

_Armor_ is built with `cmake`. To build _Armor_ tests and benchmarks create a
new directory and run `cmake`.

    mkdir build
    cd build
    cmake .. -DARMOR_INCLUDE_TESTS:BOOL=ON -DARMOR_INCLUDE_BENCHMARKS:BOOL=ON
    cmake --build .
    ctest

