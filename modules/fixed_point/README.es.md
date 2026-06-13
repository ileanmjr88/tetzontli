# Aritmética de punto fijo

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

Aritmética en formato Q y por qué evitar los flotantes en un MCU sin FPU.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`fixed_point.h` + `fixed_point.c` + `test_fixed_point.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
