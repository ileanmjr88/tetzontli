# Antirrebote de interruptores

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

Enfoques de antirrebote basados en temporizador y en registro de desplazamiento para entradas mecánicas ruidosas.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`debounce.h` + `debounce.c` + `test_debounce.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
