# Ring Buffer (FIFO circular)

[English](README.md) · **Español**

Una FIFO de bytes de un solo productor / un solo consumidor (SPSC) sobre
almacenamiento provisto por el llamador.

**[Blog](https://www.ilean.me/es/blog/patrones-embebidos-comunes-bufer-circular/)**

## Por qué importa

El ring buffer es el caballo de batalla de la E/S embebida. El uso canónico es
la UART: una interrupción se dispara con cada byte recibido y lo empuja al
búfer, mientras el bucle principal saca los bytes y los procesa cuando puede. El
productor y el consumidor corren a ritmos distintos en contextos de ejecución
distintos, y el ring buffer los desacopla sin asignación dinámica.

## Decisiones de diseño

- **Capacidad potencia de dos.** El ajuste de índices se vuelve
  `i & (capacity - 1)` en lugar de `i % capacity`, evitando una división por
  hardware que el MCU quizá no tenga.
- **Índices de libre conteo.** `head` y `tail` solo se incrementan y se
  enmascaran al acceder. La diferencia sin signo `head - tail` es el nivel de
  llenado, lo que evita la ambigüedad lleno-vs-vacío sin desperdiciar una
  posición ni un contador.
- **Almacenamiento propiedad del llamador.** Sin `malloc`. El búfer vive en
  memoria estática o de pila, que es lo que quieres en bare metal.
- **SPSC, sin bloqueos.** Con exactamente un escritor y un lector, cada índice
  tiene un único dueño, así que no se necesitan bloqueos donde las
  lecturas/escrituras de tamaño palabra son atómicas. El caso
  multiproductor/multiconsumidor queda explícitamente fuera de alcance.

## API

Ver [`ring_buffer.h`](ring_buffer.h). Operaciones principales: `init`, `put`,
`get`, `write`, `read`, además de `count` / `free` / `is_empty` / `is_full` /
`reset`.
