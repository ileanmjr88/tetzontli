# Máquina de estados finitos (basada en tablas)

[English](README.md) · **Español**

Una máquina de estados finitos basada en tablas sobre tablas constantes
provistas por el llamador.

**[Blog](https://ilean.me/es/blog/)**

## Por qué importa

Casi todo sistema embebido es una máquina de estados: un antirrebote, un driver
de módem, un cargador de batería, un bootloader. La mayoría empiezan como un
`switch(state)` con un `switch(event)` anidado en cada caso. Eso funciona con
tres estados. Con diez, el comportamiento está disperso en cientos de líneas, el
código de entrada y salida está copiado y pegado en cada caso que toca un
estado, las reglas que aplican a todos los estados no tienen un lugar natural, y
un `break` olvidado cae en silencio al siguiente estado.

Una tabla de transiciones pone todo el comportamiento en una pantalla. Cada fila
es una regla — *en `from`, ante `event`, si `guard` pasa, ejecutar `action` e ir
a `to`* — así que la tabla se lee como una especificación, y «¿qué pasa si llega
X en el estado Y?» se responde leyendo filas en lugar de rastrear ramas.

## Decisiones de diseño

- **El comportamiento son datos.** Las reglas viven en un
  `const sm_transition_t[]`, así que la tabla puede quedarse en flash. Los hooks
  `on_entry` / `on_exit` / `on_run` de cada estado viven en una segunda tabla
  opcional, así que el código de entrada y salida se escribe una vez por estado.
- **Gana la primera coincidencia.** Las filas se revisan de arriba hacia abajo.
  Las filas de seguridad con `SM_ANY_STATE` (un paro de emergencia) van primero;
  una fila con guarda va arriba de su respaldo sin guarda. El orden de las filas
  es comportamiento.
- **Las guardas son preguntas.** Una guarda fallida sigue el recorrido, que es
  como funcionan los respaldos. Las guardas se pueden ejecutar para filas que
  nunca se disparan, así que no deben tener efectos secundarios.
- **Salida, acción, entrada.** El estado viejo limpia, la transición hace su
  trabajo, y luego el estado nuevo se prepara.
- **Las autotransiciones solo ejecutan la acción.** Cambiar un punto de ajuste
  en `RUNNING` no debería detener y volver a arrancar el motor; en código
  embebido, la entrada y la salida suelen tocar hardware.
- **`on_run` devuelve un evento en lugar de despacharlo.** Un estado puede
  sondear el mundo (un sensor, un timeout, una bandera de una ISR) y reportar lo
  que pasó; la máquina despacha cuando el hook regresa, así que nunca se
  reentra a sí misma.
- **Cuatro resultados, no un `bool`.** `SM_UNHANDLED` (no existe ninguna regla)
  es distinto de `SM_GUARD_REJECTED` (existen reglas, pero todas las guardas
  dijeron que no), y ambos son distintos de `SM_ERROR` (mal uso).
- **Se valida una vez en el init.** `sm_init` revisa cada fila por adelantado y
  pone la máquina en cero antes de validar, así que un init fallido deja una
  máquina que `sm_start`, `sm_dispatch` y `sm_run` rechazan. `sm_init` no
  ejecuta código del usuario; `sm_start` entra al estado inicial.
- **Estados y eventos de `uint8_t`.** Las filas se mantienen chicas, a costa de
  un límite de 255 de cada uno; `0xFF` está reservado para `SM_ANY_STATE` y
  `SM_NO_EVENT`.
- **`void *ctx` en todos lados.** Todo callback recibe el contexto del llamador,
  así que no hay variables globales, se pueden ejecutar varias máquinas a la
  vez, y las pruebas pueden pasar un contexto falso.
- **No es segura ante ISR ni reentrante.** Las guardas, las acciones y los hooks
  no deben llamar a `sm_dispatch` ni a `sm_run` sobre la misma máquina. Encola
  los eventos de una ISR (un `ring_buffer` transporta eventos de un byte tal
  cual) y despáchalos desde un solo contexto.
- **Plana, con recorrido lineal.** No hay estados jerárquicos; el comportamiento
  compartido entre estados va en filas `SM_ANY_STATE`. Cada despacho recorre la
  tabla, lo cual está bien para decenas de filas.

## API

Ver [`state_machine.h`](state_machine.h). Operaciones principales: `init`,
`start`, `dispatch`, `run`, además de `state`. Declara la tabla de hooks como
`hooks[STATE_COUNT]`, con `STATE_COUNT` como el último valor del `enum` de
estados: `sm_init` solo recibe un puntero y no puede verificar su longitud, pero
el compilador marca las entradas de más y rellena las que faltan con `NULL`
(sin hook). La suite de pruebas arma un controlador de motor completo y sirve
además como ejemplo de uso.
