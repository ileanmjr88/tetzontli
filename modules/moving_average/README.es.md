# Media móvil / filtro paso bajo

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

Suavizado de lecturas ruidosas del ADC; media por ventana apoyada en un ring buffer y una variante EWMA.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`moving_average.h` + `moving_average.c` + `test_moving_average.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
