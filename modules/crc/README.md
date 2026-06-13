# CRC Checksum

**English** · [Español](README.es.md)

> **Status:** planned — not yet implemented.

Table-driven CRC-8 / CRC-16; used by the frame parser for integrity checks.

**[Blog](https://ilean.me/blog/)**

When built, this module follows the same layout as `ring_buffer/`:
`crc.h` + `crc.c` + `test_crc.cpp` + `CMakeLists.txt`, host-tested
with GoogleTest and wired into the root build via `add_subdirectory`.
