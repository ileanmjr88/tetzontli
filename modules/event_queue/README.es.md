# Cola de eventos / mensajes

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

Despacho de eventos productor/consumidor construido sobre el ring buffer; distribución a los manejadores registrados.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`event_queue.h` + `event_queue.c` + `test_event_queue.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
