# Temporizadores por software / planificador cooperativo

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

Retardos no bloqueantes y un planificador cooperativo dirigido por ticks a partir de una sola interrupción de temporizador.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`soft_timer.h` + `soft_timer.c` + `test_soft_timer.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
