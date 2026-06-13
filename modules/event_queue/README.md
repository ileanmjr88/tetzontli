# Event / Message Queue

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

Producer/consumer event dispatch built on the ring buffer; fan-out to registered handlers.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`event_queue.h` + `event_queue.c` + `test_event_queue.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
