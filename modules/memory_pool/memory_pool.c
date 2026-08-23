#include <stdint.h>
#include <string.h>
#include "memory_pool.h"

static bool is_power_of_two(size_t x) {
  return x != 0u && (x & (x - 1u)) == 0u;
}

static size_t round_up(size_t x, size_t a) {
  return (x + (a - 1u)) & ~(a - 1u);
}

static void *read_next(const void *block) {
  void *const *slot = (void *const *)block;
  void *next;
  memcpy(&next, slot, sizeof next);
  return next;
}

static void write_next(void *block, void *next) {
  void **slot = (void **)block;
  memcpy(slot, &next, sizeof next);
}

void memory_pool_reset(memory_pool_t *mp) {
  if (mp == NULL) {
    return;
  }

  mp->free_head = NULL;
  for (size_t i = mp->capacity; i-- > 0;) {
    uint8_t *blk = mp->base + i * mp->block_size;
    write_next(blk, mp->free_head);
    mp->free_head = blk;
  }
  mp->used = 0u;
}

bool memory_pool_init(memory_pool_t *mp, void *storage, size_t storage_size, size_t block_size, size_t block_align) {
  if (mp == NULL || storage == NULL || !is_power_of_two(block_align) || block_size < sizeof(void *)) {
    return false;
  }

  if (block_align < _Alignof(void *)) {
    block_align = _Alignof(void *);
  }

  size_t stride = round_up(block_size, block_align);
  if (stride < block_size) {
    return false;
  }

  uintptr_t addr = (uintptr_t)storage;
  uintptr_t aligned = (uintptr_t)round_up((size_t)addr, block_align);
  uint8_t *base = (uint8_t *)aligned;

  size_t offset = (size_t)(base - (uint8_t *)storage);
  if (offset >= storage_size) {
    return false;
  }

  size_t usable = storage_size - offset;
  size_t capacity = usable / stride;
  if (capacity == 0u) {
    return false;
  }

  mp->base = base;
  mp->block_size = stride;
  mp->capacity = capacity;
  mp->used = 0u;
  mp->high_water = 0u;
  memory_pool_reset(mp);
  return true;
}

size_t memory_pool_capacity(const memory_pool_t *mp) {
  return (mp == NULL) ? 0u : mp->capacity;
}

size_t memory_pool_used(const memory_pool_t *mp) {
  return (mp == NULL) ? 0u : mp->used;
}

size_t memory_pool_available(const memory_pool_t *mp) {
  return (mp == NULL) ? 0u : mp->capacity - mp->used;
}

size_t memory_pool_high_water(const memory_pool_t *mp) {
  return (mp == NULL) ? 0u : mp->high_water;
}

bool memory_pool_owns(const memory_pool_t *mp, const void *p) {
  if (mp == NULL || p == NULL) {
    return false;
  }

  uintptr_t addr = (uintptr_t)p;
  uintptr_t base = (uintptr_t)mp->base;
  uintptr_t end = base + ((uintptr_t)mp->capacity * (uintptr_t)mp->block_size);

  if (addr < base || addr >= end){
    return false;
  }
  return (addr - base) % (uintptr_t)mp->block_size == 0u;
}

void *memory_pool_alloc(memory_pool_t *mp) {
  if (mp == NULL || mp->free_head == NULL) {
    return NULL;
  }

  void *p = mp->free_head;
  mp->free_head = read_next(p);
  mp->used++;
  if (mp->used > mp->high_water) {
    mp->high_water = mp->used;
  }
  return p;
}

bool memory_pool_free(memory_pool_t *mp, void *block) {
  if (!memory_pool_owns(mp, block)) {
    return false;
  }
  if (mp->used == 0u) {
    return false;
  }

  write_next(block, mp->free_head);
  mp->free_head = block;
  mp->used--;
  return true;
}


