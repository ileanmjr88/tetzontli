# Memory Pool (asignador de bloques fijos)

[English](README.md) · **Español**

Un asignador de bloques de tamaño fijo sobre almacenamiento provisto por el
llamador.

**[Blog](https://www.ilean.me/es/blog/patrones-embebidos-comunes-pool-de-memoria/)**

## Por qué importa

`malloc` en un sistema embebido de larga ejecución falla de una forma difícil de
probar: cada asignación de un tamaño distinto deja un hueco, los huecos nunca
encajan del todo con la siguiente petición y, tras días encendido, al heap le
sobran bytes libres pero ya no le queda un tramo contiguo que entregar. Las
asignaciones empiezan a fallar en un dispositivo que llevaba una semana
funcionando bien.

Un pool de tamaño fijo elimina ese modo de fallo por construcción. Todos los
bloques miden lo mismo, así que cualquier bloque libre sirve para cualquier
petición y no hay nada que fragmentar. Reservar y liberar son un par de
asignaciones de puntero cada uno — tiempo acotado, que importa tanto como la
velocidad media cuando hay un plazo de por medio.

## Decisiones de diseño

- **Solo bloques de tamaño fijo.** La asignación de tamaño variable reintroduce
  fragmentación, fusión de huecos y búsqueda — justo el fallo que este patrón
  existe para evitar.
- **Lista de libres intrusiva.** Un bloque libre guarda el puntero `next` en sus
  propios primeros bytes. Un bloque está en la lista o es del llamador, nunca
  las dos cosas, así que la sobrecarga por bloque es literalmente cero.
- **Una pila, no una cola.** `free` empuja a la cabeza y `alloc` la saca, lo que
  deja ambas en O(1) sin recorrer nada. Devolver primero el bloque liberado más
  recientemente es además lo más amable con la caché.
- **Almacenamiento propiedad del llamador.** Sin `malloc`. `init` alinea la
  arena hacia arriba internamente y absorbe los bytes desperdiciados en lugar de
  rechazar un búfer incómodo.
- **`owns()` conserva el módulo.** A diferencia de `ring_buffer`, redondear a
  potencia de dos aquí inflaría *cada bloque* en vez de una sola capacidad: un
  registro de 40 bytes costaría 64. La división vale la RAM, y además se ejecuta
  una vez por objeto en lugar de una vez por byte.
- **No es segura ante ISR.** Toda llamada que muta toca una única lista de
  libres compartida. Envuélvelas en una sección crítica; aquí no hay camino sin
  bloqueos como en `ring_buffer`.
- **Los bloques no se ponen a cero.** Semántica de `malloc`, no de `calloc`. Un
  bloque recién asignado contiene un puntero obsoleto de la lista de libres y lo
  que dejara su dueño anterior.

## API

Ver [`memory_pool.h`](memory_pool.h). Operaciones principales: `init`, `alloc`,
`free`, `reset`, además de `capacity` / `used` / `available` / `owns` /
`high_water`. `high_water` es el pico desde `init` y sobrevive a `reset`: es el
número con el que dimensionas el pool.
