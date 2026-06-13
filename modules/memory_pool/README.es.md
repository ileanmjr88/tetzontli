# Asignador estático de bloques fijos

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

Asignación sin malloc desde una arena de tamaño predefinido; gestión de lista de libres; por qué la fragmentación del heap mata a los sistemas embebidos de larga ejecución.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`memory_pool.h` + `memory_pool.c` + `test_memory_pool.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
