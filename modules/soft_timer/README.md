# Software Timers / Cooperative Scheduler

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

Non-blocking delays and a tick-driven cooperative scheduler from a single timer interrupt.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`soft_timer.h` + `soft_timer.c` + `test_soft_timer.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
