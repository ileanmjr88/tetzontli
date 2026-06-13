
#include "ring_buffer.h"
static bool is_power_of_two(size_t x) {
  return x != 0u && (x & (x - 1u)) == 0u;
}

bool ring_buffer_init(ring_buffer_t *rb, uint8_t *storage, size_t capacity) {
  if (rb == NULL || storage == NULL || !is_power_of_two(capacity)) {
    return false;
  }

  rb->buffer = storage;
  rb->capacity = capacity;
  rb->mask = capacity - 1u;
  rb->head = 0u;
  rb->tail = 0u;
  return true;
}

size_t ring_buffer_count(const ring_buffer_t *rb) {
  return rb->head - rb->tail;
}

size_t ring_buffer_free(const ring_buffer_t *rb) {
  return rb->capacity - ring_buffer_count(rb);
}

bool ring_buffer_is_empty(const ring_buffer_t *rb) {
  return rb->head == rb->tail;
}

bool ring_buffer_is_full(const ring_buffer_t *rb) {
  return ring_buffer_count(rb) == rb->capacity;
}

void ring_buffer_reset(ring_buffer_t *rb) {
  rb->head = 0u;
  rb->tail = 0u;
}

bool ring_buffer_put(ring_buffer_t *rb, uint8_t byte) {
  if (rb == NULL || ring_buffer_is_full(rb)) {
    return false;
  }
  rb->buffer[rb->head & rb->mask] = byte;
  rb->head++;
  return true;
}

bool ring_buffer_get(ring_buffer_t *rb, uint8_t *out) {
  if (rb == NULL || out == NULL || ring_buffer_is_empty(rb)) {
    return false;
  }
  *out = rb->buffer[rb->tail & rb->mask];
  rb->tail++;
  return true;
}

size_t ring_buffer_write(ring_buffer_t *rb, const uint8_t *data, size_t len) {
  if (rb == NULL || data == NULL) {
    return 0u;
  }
  size_t written = 0u;
  while (written < len && ring_buffer_put(rb, data[written])) {
    written++;
  }
  return written;
}

size_t ring_buffer_read(ring_buffer_t *rb, uint8_t *out, size_t len) {
  if (rb == NULL || out == NULL) {
    return 0u;
  }
  size_t count = 0u;
  while (count < len && ring_buffer_get(rb, &out[count])) {
    count++;
  }
  return count;
}
