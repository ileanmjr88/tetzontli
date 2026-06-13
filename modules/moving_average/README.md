# Moving-Average / Low-Pass Filter

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

Smoothing noisy ADC readings; ring-buffer-backed windowed average and an EWMA variant.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`moving_average.h` + `moving_average.c` + `test_moving_average.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
