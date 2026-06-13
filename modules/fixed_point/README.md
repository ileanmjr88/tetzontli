# Fixed-Point Math

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

Q-format arithmetic and why you avoid floats on an MCU without an FPU.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`fixed_point.h` + `fixed_point.c` + `test_fixed_point.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
