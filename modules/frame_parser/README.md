# Framing / Packet Parser

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

Start/end delimiters, byte-stuffing, and checksum validation, implemented as a state machine.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`frame_parser.h` + `frame_parser.c` + `test_frame_parser.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
