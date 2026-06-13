# Tetzontli

[English](README.md) · **Español**

> *Tetzontli* — la piedra volcánica de cimentación sobre la que construían los
> aztecas. Un nombre apropiado para una biblioteca de los patrones fundamentales
> sobre los que se construye firmware.

Una biblioteca de patrones comunes de software embebido, cada uno implementado
en **C** (para que se integre directamente en un proyecto bare-metal) y
**probado en el host con GoogleTest** (C++), usando las mismas herramientas de
CMake + Ninja + sanitizers + CI que un código de producción.

Cada patrón es un módulo autocontenido dentro de [`modules/`](modules/) — una
carpeta, una cabecera, una implementación, un conjunto de pruebas — y respalda
una entrada de la serie de blog *Common Embedded Patterns*.

## ¿Por qué implementaciones en C y pruebas en C++?

Estos patrones están pensados para correr en microcontroladores sin heap, sin
STL y a menudo sin FPU. Escribirlos en C puro los mantiene honestos y portables
al hardware real. El arnés de pruebas, en cambio, corre en tu máquina host,
donde C++ y GoogleTest ofrecen una forma rápida y expresiva de verificar el
comportamiento — incluyendo casos límite difíciles de reproducir en el destino.
Las fuentes en C se envuelven con `extern "C"` y se enlazan dentro de los
binarios de prueba en C++.

## Los patrones

| # | Módulo | Patrón | Estado |
|---|--------|--------|--------|
| 1 | [`ring_buffer`](modules/ring_buffer) | FIFO circular, SPSC, segura ante ISR | ✅ hecho |
| 2 | [`memory_pool`](modules/memory_pool) | Asignador estático de bloques fijos | 📋 planeado |
| 3 | [`state_machine`](modules/state_machine) | Máquina de estados finita | 📋 planeado |
| 4 | [`command_dispatch`](modules/command_dispatch) | Tabla de despacho de comandos | 📋 planeado |
| 5 | [`event_queue`](modules/event_queue) | Cola de eventos / mensajes | 📋 planeado |
| 6 | [`soft_timer`](modules/soft_timer) | Temporizadores por software / planificador | 📋 planeado |
| 7 | [`debounce`](modules/debounce) | Antirrebote de interruptores | 📋 planeado |
| 8 | [`moving_average`](modules/moving_average) | Media móvil / filtro paso bajo | 📋 planeado |
| 9 | [`frame_parser`](modules/frame_parser) | Analizador de tramas / paquetes | 📋 planeado |
| 10 | [`crc`](modules/crc) | Suma de verificación CRC-8 / CRC-16 | 📋 planeado |
| 11 | [`fixed_point`](modules/fixed_point) | Aritmética de punto fijo formato Q | 📋 planeado |
| 12 | [`bit_ops`](modules/bit_ops) | Abstracción de registros / campos de bits | 📋 planeado |

El orden es deliberado — los módulos posteriores reutilizan los anteriores (el
despachador de comandos consume el ring buffer, el analizador de tramas es una
máquina de estados que llama al módulo CRC, el filtro de media móvil se apoya en
un ring buffer).

## Inicio rápido

La cadena de herramientas está fijada con [Compendium](https://compendium.ilean.me)
— las mismas versiones de clang, CMake y Ninja en cada máquina, sin Docker, a
velocidad nativa.

```bash
# Instalar la cadena de herramientas fijada (clang, cmake, ninja, ccache)
compendium install

# Activarla para esta shell
source <(compendium activate)

# Configurar, compilar y probar con la cadena gestionada
cmake --preset compendium
cmake --build build
ctest --test-dir build --output-on-failure
```

¿No tienes Compendium? Instálalo con
`curl -fsSL https://compendium.ilean.me/install.sh | sh`, o compila con tu propio
clang/cmake/ninja a la manera tradicional:
`cmake -B build -G Ninja && cmake --build build`.

### Opciones de compilación

Un segundo preset activa los sanitizers:

```bash
cmake --preset sanitizers   # AddressSanitizer + UBSan
cmake --build build
ctest --test-dir build --output-on-failure
```

O pasa opciones directamente a cualquier configuración:

```bash
cmake --preset compendium \
  -DENABLE_COVERAGE=ON \     # instrumentación de cobertura
  -DENABLE_CLANG_TIDY=ON \   # clang-tidy en cada fuente C
  -DENABLE_CPPCHECK=ON       # cppcheck en cada fuente C
```

## Agregar un módulo

1. Crea `modules/<nombre>/` con `<nombre>.h`, `<nombre>.c`,
   `test_<nombre>.cpp` y un `CMakeLists.txt` (copia
   `ring_buffer/CMakeLists.txt` y renómbralo).
2. Descomenta la línea `add_subdirectory(modules/<nombre>)` en el
   `CMakeLists.txt` raíz.
3. Compila y prueba.

## Traducciones al español

Los textos en español (`README.es.md` en cada módulo) son traducciones mías.
**No busco contribuciones de código** en este proyecto, pero si detectas un error
de traducción o algo que suene raro, te agradecería el aviso —
[abre una incidencia de traducción](../../issues/new?template=spanish-translation.yml).

## Licencia

MIT (ver [LICENSE](LICENSE)).
