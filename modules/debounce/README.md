# Switch Debouncing

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

Timer-based and shift-register debounce approaches for noisy mechanical inputs.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`debounce.h` + `debounce.c` + `test_debounce.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
