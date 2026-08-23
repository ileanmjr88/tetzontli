/**
 * @file memory_pool.h
 * @brief Fixed-size block allocator over caller-supplied storage.
 *
 * Carves a caller-owned buffer into equal-sized blocks and hands them out in
 * O(1) with no fragmentation by construction. Free blocks are threaded onto an
 * intrusive LIFO free list stored inside the blocks themselves, so per-block
 * overhead is zero and no dynamic allocation happens anywhere. See README.md
 * for the design rationale.
 *
 * The pool is an allocator, not a container: it never reads or writes the
 * caller's data, it only hands out a place to put it. Type knowledge lives
 * only in memory_pool_init(), via sizeof(T) and _Alignof(T).
 *
 * Contract:
 *
 * - Not ISR-safe and not thread-safe. Every mutating call touches the shared
 *   free list; wrap them in a critical section if more than one context uses
 *   the same pool. Unlike ring_buffer, there is no lock-free path here.
 *
 * - Blocks are not zeroed. A freshly allocated block holds a stale free-list
 *   pointer in its first sizeof(void *) bytes and whatever the previous owner
 *   left in the rest. These are malloc semantics, not calloc.
 *
 * - Every function is NULL-safe. The queries return 0 on a NULL pool.
 *
 * - Double-free is only partly detectable. Freeing into an empty pool is
 *   rejected, but freeing a block that is already on the free list looks
 *   exactly like a valid free: it links the block to itself, and the next two
 *   allocations then hand the same block to two different callers. Do not rely
 *   on the return value of memory_pool_free() to catch that case.
 */

#ifndef TETZONTLI_MEMORY_POOL_H
#define TETZONTLI_MEMORY_POOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Pool handle. Fields are readable for inspection but must not be written. */
typedef struct {
  uint8_t *base;       /**< First block: storage rounded up to block_align.  */
  size_t   block_size; /**< Stride: requested size rounded up to alignment.  */
  size_t   capacity;   /**< Blocks carved out of the storage.                */
  size_t   used;       /**< Blocks currently handed out.                     */
  size_t   high_water; /**< Peak used since init; survives reset.            */
  void    *free_head;  /**< Head of the intrusive free list, or NULL.        */
} memory_pool_t;

/**
 * @brief Carve @p storage into fixed-size blocks and build the free list.
 *
 * The stride is @p block_size rounded up to @p block_align, and the first block
 * starts at @p storage rounded up to that same alignment. Both the lead-in
 * bytes lost to that bump and the unusable tail remainder are absorbed, so the
 * resulting capacity may be smaller than @p storage_size / @p block_size — read
 * it back with memory_pool_capacity() rather than assuming. @p block_align is
 * clamped up to _Alignof(void *), since every free block stores a pointer in
 * its first bytes.
 *
 * @param mp           Pool to initialize.
 * @param storage      Caller-owned backing buffer; must outlive the pool.
 * @param storage_size Size of @p storage in bytes.
 * @param block_size   Bytes needed per block; must be >= sizeof(void *).
 * @param block_align  Required block alignment; must be a power of two, so 0 is
 *                     rejected. Pass _Alignof(T) for a pool of T.
 * @return true on success. false if either pointer is NULL, @p block_align is
 *         not a power of two, @p block_size is smaller than a pointer, or the
 *         storage cannot hold at least one aligned block.
 */
bool   memory_pool_init(memory_pool_t *mp, void *storage, size_t storage_size,
  size_t block_size, size_t block_align);

/**
 * @brief Take one block from the pool.
 *
 * @return Pointer to a block of at least the requested size, aligned as asked
 *         for at init, with unspecified contents — see the not-zeroed note
 *         above. NULL if the pool is exhausted or @p mp is NULL.
 */
void  *memory_pool_alloc(memory_pool_t *mp);

/**
 * @brief Return a block to the pool.
 *
 * A rejected call leaves the pool completely untouched, free list included.
 * See the double-free note above for the one case this cannot reject.
 *
 * @return true if the block was returned. false if @p mp is NULL, @p block is
 *         not the start of a block in this pool, or nothing is outstanding.
 */
bool   memory_pool_free(memory_pool_t *mp, void *block);

/**
 * @brief Return every block to the pool, rebuilding the free list in address
 *        order.
 *
 * Invalidates every outstanding pointer. high_water deliberately persists: it
 * is the peak since init, which is the number you size the pool from. No-op on
 * a NULL pool.
 */
void   memory_pool_reset(memory_pool_t *mp);

/** @return Total blocks in the pool, or 0 if @p mp is NULL. */
size_t memory_pool_capacity(const memory_pool_t *mp);

/** @return Blocks currently handed out, or 0 if @p mp is NULL. */
size_t memory_pool_used(const memory_pool_t *mp);

/**
 * @brief Blocks still free, i.e. capacity minus used.
 *
 * Named available() rather than free() because memory_pool_free() already
 * means "give this block back."
 *
 * @return Free block count, or 0 if @p mp is NULL.
 */
size_t memory_pool_available(const memory_pool_t *mp);

/**
 * @brief Test whether @p p is the start of a block belonging to this pool.
 *
 * True only for exact block starts. A pointer into the middle of a block lies
 * inside the arena but is not a block, and is rejected. This is the same check
 * memory_pool_free() applies.
 *
 * @return true if @p p is a block start in this pool, false otherwise or if
 *         either pointer is NULL.
 */
bool   memory_pool_owns(const memory_pool_t *mp, const void *p);

/**
 * @brief Peak simultaneous usage since init, for sizing the pool.
 *
 * Unaffected by memory_pool_free() and memory_pool_reset(); only a fresh
 * memory_pool_init() clears it.
 *
 * @return Peak used count, or 0 if @p mp is NULL.
 */
size_t memory_pool_high_water(const memory_pool_t *mp);

#ifdef __cplusplus
}
#endif

#endif // TETZONTLI_MEMORY_POOL_H
