# Suma de verificación CRC

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

CRC-8 / CRC-16 dirigido por tablas; usado por el analizador de tramas para comprobaciones de integridad.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`crc.h` + `crc.c` + `test_crc.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
