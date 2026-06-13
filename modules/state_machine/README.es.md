# Máquina de estados finita

[English](README.md) · **Español**

> **Estado:** planeado — aún no implementado.

Máquinas de estados dirigidas por tablas frente a las basadas en switch; tablas de transición; se conecta con el manejo de comandos del intérprete de comandos.

**[Blog](https://ilean.me/es/blog/)**

Cuando se construya, este módulo seguirá el mismo esquema que `ring_buffer/`:
`state_machine.h` + `state_machine.c` + `test_state_machine.cpp` + `CMakeLists.txt`, probado en el
host con GoogleTest e integrado en la compilación raíz vía `add_subdirectory`.
