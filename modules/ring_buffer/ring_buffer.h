/**
 * @file ring_buffer.h
 * @brief Single-producer / single-consumer byte FIFO over caller-supplied storage.
 *
 * Lock-free circular buffer with power-of-two capacity and free-running indices;
 * no dynamic allocation — the caller owns the backing storage. See README.md for
 * the design rationale.
 */

#ifndef TETZONTLI_RING_BUFFER_H
#define TETZONTLI_RING_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint8_t        *buffer;
  size_t          capacity;
  size_t          mask;
  volatile size_t head;
  volatile size_t tail;
  
} ring_buffer_t;

bool ring_buffer_init(ring_buffer_t *rb, uint8_t *storage, size_t capacity);
size_t ring_buffer_count(const ring_buffer_t *rb);
size_t ring_buffer_free(const ring_buffer_t *rb);
bool ring_buffer_is_empty(const ring_buffer_t *rb);
bool ring_buffer_is_full(const ring_buffer_t *rb);
void ring_buffer_reset(ring_buffer_t *rb);
bool ring_buffer_put(ring_buffer_t *rb, uint8_t byte);
bool ring_buffer_get(ring_buffer_t *rb, uint8_t *out);
size_t ring_buffer_write(ring_buffer_t *rb, const uint8_t *data, size_t len);
size_t ring_buffer_read(ring_buffer_t *rb, uint8_t *out, size_t len);

#ifdef __cplusplus
}
#endif

#endif // TETZONTLI_RING_BUFFER_H

