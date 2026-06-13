# Tabla de despacho de comandos

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

Tablas de búsqueda de punteros a función para registrar e invocar manejadores de comandos; consume el ring buffer como entrada.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`command_dispatch.h` + `command_dispatch.c` + `test_command_dispatch.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
