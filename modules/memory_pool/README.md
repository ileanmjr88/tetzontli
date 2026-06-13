# Fixed-Block Static Allocator

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

malloc-free allocation from a pre-sized arena; free-list management; why heap fragmentation kills long-running embedded systems.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`memory_pool.h` + `memory_pool.c` + `test_memory_pool.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
