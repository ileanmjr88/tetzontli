# Ring Buffer (Circular FIFO)

**English** · [Español](README.es.md)

A single-producer / single-consumer byte FIFO over caller-supplied storage.

**[Blog](https://www.ilean.me/blog/common-embedded-patterns-ring-buffer/)**

## Why it matters

The ring buffer is the workhorse of embedded I/O. The canonical use is UART:
an interrupt fires on each received byte and pushes it into the buffer, while
the main loop pulls bytes out and processes them when it gets around to it.
Producer and consumer run at different rates in different execution contexts,
and the ring buffer decouples them without dynamic allocation.

## Design decisions

- **Power-of-two capacity.** Index wrapping becomes `i & (capacity - 1)`
  instead of `i % capacity`, avoiding a hardware divide the MCU may not have.
- **Free-running indices.** `head` and `tail` only increment and are masked on
  access. The unsigned difference `head - tail` is the fill level, which
  sidesteps the full-vs-empty ambiguity without wasting a slot or a counter.
- **Caller-owned storage.** No `malloc`. The buffer lives in static or stack
  memory, which is what you want on bare metal.
- **SPSC, lock-free.** With exactly one writer and one reader, each index has a
  single owner, so no locking is needed where word-sized loads/stores are
  atomic. Multi-producer/consumer is explicitly out of scope.

## API

See [`ring_buffer.h`](ring_buffer.h). Core operations: `init`, `put`, `get`,
`write`, `read`, plus `count` / `free` / `is_empty` / `is_full` / `reset`.
