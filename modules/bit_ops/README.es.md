# Manipulación de bits / abstracción de registros

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

Acceso a registros con seguridad de tipos y utilidades de campos de bits con costo cero en tiempo de ejecución.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`bit_ops.h` + `bit_ops.c` + `test_bit_ops.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
