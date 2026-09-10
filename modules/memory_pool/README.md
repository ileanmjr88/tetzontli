# Memory Pool (Fixed-Block Allocator)

**English** · [Español](README.es.md)

A fixed-size block allocator over caller-supplied storage.

**[Blog](https://www.ilean.me/blog/common-embedded-patterns-memory-pool/)**

## Why it matters

`malloc` on a long-running embedded system fails in a way that is hard to test
for: every allocation of a different size leaves a hole, the holes never quite
fit the next request, and after days of uptime the heap has plenty of free bytes
but no contiguous run left to hand out. Allocation starts failing on a device
that has been running fine for a week.

A fixed-size pool removes that failure mode by construction. Every block is the
same size, so any free block satisfies any request and there is nothing to
fragment. Allocation and release are each a couple of pointer assignments —
bounded time, which matters as much as average speed when there is a deadline
involved.

## Design decisions

- **Fixed-size blocks only.** Variable-size allocation reintroduces
  fragmentation, coalescing, and search — the exact failure this pattern exists
  to avoid.
- **Intrusive free list.** A free block stores the `next` pointer in its own
  first bytes. A block is either on the list or owned by the caller, never both,
  so per-block overhead is genuinely zero.
- **A stack, not a queue.** `free` pushes onto the head and `alloc` pops it,
  making both O(1) with no traversal. Handing back the most recently freed block
  is also the cache-friendly choice.
- **Caller-owned storage.** No `malloc`. `init` aligns the arena up internally
  and absorbs the wasted bytes rather than rejecting an awkward buffer.
- **`owns()` keeps the modulus.** Unlike `ring_buffer`, rounding to a power of
  two here would inflate *every block* rather than one capacity — a 40-byte
  record would cost 64. The division is worth the RAM, and it runs once per
  object instead of once per byte.
- **Not ISR-safe.** Every mutating call touches one shared free list. Wrap them
  in a critical section; there is no lock-free path as there is in
  `ring_buffer`.
- **Blocks are not zeroed.** `malloc` semantics, not `calloc`. A fresh block
  holds a stale free-list pointer and whatever the previous owner left behind.

## API

See [`memory_pool.h`](memory_pool.h). Core operations: `init`, `alloc`, `free`,
`reset`, plus `capacity` / `used` / `available` / `owns` / `high_water`.
`high_water` is the peak since `init` and survives `reset` — it is the number
you size the pool from.
