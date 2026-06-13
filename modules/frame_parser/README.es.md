# Analizador de tramas / paquetes

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

Delimitadores de inicio/fin, byte-stuffing y validación de suma de verificación, implementado como una máquina de estados.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`frame_parser.h` + `frame_parser.c` + `test_frame_parser.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
