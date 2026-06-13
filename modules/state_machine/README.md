# Finite State Machine

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

Table-driven vs switch-based FSMs; transition tables; ties back to the command interpreter's command handling.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`state_machine.h` + `state_machine.c` + `test_state_machine.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
