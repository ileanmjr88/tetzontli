# Bit Manipulation / Register Abstraction

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

Type-safe register access and bitfield helpers with zero runtime cost.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`bit_ops.h` + `bit_ops.c` + `test_bit_ops.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
