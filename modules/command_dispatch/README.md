# Command Dispatch Table

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

Function-pointer lookup tables for registering and invoking command handlers; consumes the ring buffer for input.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`command_dispatch.h` + `command_dispatch.c` + `test_command_dispatch.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
