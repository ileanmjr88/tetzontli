#include <cstdint>
#include <cstddef>
#include <array>
#include <cstring>
#include <set>
#include <vector>
#include <gtest/gtest.h>

extern "C" {
#include "memory_pool.h"
}

namespace {

class MemoryPoolTest : public ::testing::Test {
protected:
  static constexpr size_t kBlockSize = 12; // what the caller asks for
  static constexpr size_t kBlockAlign = 8; // >= alignof(void *)
  static constexpr size_t kStride = 16;    // 12 rounded up to 8
  static constexpr size_t kCapacity = 8;
  static constexpr size_t kStorageSize = kStride * kCapacity; // 128

  alignas(16) uint8_t storage[kStorageSize]{};
  memory_pool_t mp{};

  void SetUp() override {
    ASSERT_TRUE(memory_pool_init(&mp, storage, sizeof storage, kBlockSize, kBlockAlign));
    ASSERT_EQ(memory_pool_capacity(&mp), kCapacity);
  }

  std::array<void *, kCapacity> AllocAll() {
    std::array<void *, kCapacity> blocks{};
    for(size_t i = 0; i < kCapacity; ++i) {
      blocks[i] = memory_pool_alloc(&mp);
    }
   return blocks;
  }
};

uint32_t NextRandom(uint32_t &state) {
  state = state * 1664525u + 1013904223u;
  return state;
}

TEST_F(MemoryPoolTest, InitRejectsNullPoolOrStorage) {
  memory_pool_t bad{};
  alignas(16) uint8_t buf[kStorageSize]{};

  EXPECT_FALSE(memory_pool_init(nullptr, buf, sizeof buf, kBlockSize, kBlockAlign));
  EXPECT_FALSE(memory_pool_init(&bad, nullptr, sizeof buf, kBlockSize, kBlockAlign));

  EXPECT_EQ(memory_pool_capacity(&bad), 0u);
  EXPECT_EQ(memory_pool_alloc(&bad), nullptr);
}

TEST_F(MemoryPoolTest, InitRejectsNonPowerOfTwoAlign) {
  memory_pool_t bad{};
  alignas(16) uint8_t buf[kStorageSize]{};

  EXPECT_FALSE(memory_pool_init(&bad, buf, sizeof buf, kBlockSize, 0));
  EXPECT_FALSE(memory_pool_init(&bad, buf, sizeof buf, kBlockSize, 6));
  EXPECT_FALSE(memory_pool_init(&bad, buf, sizeof buf, kBlockSize, 12));

  EXPECT_EQ(memory_pool_capacity(&bad), 0u);
}

TEST_F(MemoryPoolTest, InitRejectsBlockSmallerThanPointer){
  memory_pool_t bad{};
  alignas(16) uint8_t buf[kStorageSize]{};

  EXPECT_FALSE(memory_pool_init(&bad, buf, sizeof buf, 0, kBlockAlign));
  EXPECT_FALSE(memory_pool_init(&bad, buf, sizeof buf, 1, kBlockAlign));
  EXPECT_FALSE(memory_pool_init(&bad, buf, sizeof buf, sizeof(void *) - 1, kBlockAlign));

  memory_pool_t good{};
  EXPECT_TRUE(memory_pool_init(&good, buf, sizeof buf, sizeof(void *), kBlockAlign));
}

TEST_F(MemoryPoolTest, InitRejectsStorageTooSmall){
  memory_pool_t bad{};
  alignas(16) uint8_t buf[kStorageSize]{};

  EXPECT_FALSE(memory_pool_init(&bad, buf, 0, kBlockSize, kBlockAlign));
  EXPECT_FALSE(memory_pool_init(&bad, buf, kStride - 1, kBlockSize, kBlockAlign));
  EXPECT_EQ(memory_pool_capacity(&bad), 0u);

  memory_pool_t good{};
  EXPECT_TRUE(memory_pool_init(&good, buf, sizeof buf, kBlockSize, kBlockAlign));
  EXPECT_EQ(memory_pool_capacity(&good), 8u);
}

TEST_F(MemoryPoolTest, InitRejectsWhenAlignmentBumpEatsArena) {
  memory_pool_t bad{};
  alignas(16) uint8_t buf[kStorageSize]{};

  uint8_t *unaligned = buf + 1;
  ASSERT_EQ(reinterpret_cast<uintptr_t>(unaligned) % kBlockAlign, 1u);

  EXPECT_FALSE(memory_pool_init(&bad, unaligned, 4, kBlockSize, kBlockAlign));
  EXPECT_FALSE(memory_pool_init(&bad, unaligned, 7, kBlockSize, kBlockAlign));

  EXPECT_EQ(memory_pool_capacity(&bad), 0u);
  EXPECT_EQ(memory_pool_alloc(&bad), nullptr);
}

// ---- Init: derivation ---------------------------------------------------

TEST_F(MemoryPoolTest, InitRoundsStrideUpToAlignment) {
  memory_pool_t pool{};
  alignas(16) uint8_t buf[kStorageSize]{};

  ASSERT_TRUE(memory_pool_init(&pool, buf, sizeof buf, 12, 8));
  EXPECT_EQ(pool.block_size, 16u);  // the stride, not the requested 12

  // Observable without reading the struct: consecutive blocks sit one stride
  // apart, not one requested-size apart.
  auto *first = static_cast<uint8_t *>(memory_pool_alloc(&pool));
  auto *second = static_cast<uint8_t *>(memory_pool_alloc(&pool));
  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(second - first, 16);

  // An already-aligned request is left alone.
  ASSERT_TRUE(memory_pool_init(&pool, buf, sizeof buf, 8, 8));
  EXPECT_EQ(pool.block_size, 8u);
  EXPECT_EQ(memory_pool_capacity(&pool), 16u);
}

TEST_F(MemoryPoolTest, InitTruncatesCapacityOnInexactDivision) {
  memory_pool_t pool{};
  alignas(16) uint8_t buf[kStorageSize]{};

  // Exact: 128 / 16.
  ASSERT_TRUE(memory_pool_init(&pool, buf, kStorageSize, kBlockSize, kBlockAlign));
  EXPECT_EQ(memory_pool_capacity(&pool), 8u);

  // Inexact: leftover bytes are dropped, never rounded up into a block.
  ASSERT_TRUE(memory_pool_init(&pool, buf, kStorageSize - 1, kBlockSize, kBlockAlign));
  EXPECT_EQ(memory_pool_capacity(&pool), 7u);

  ASSERT_TRUE(memory_pool_init(&pool, buf, kStride * 3 + 1, kBlockSize, kBlockAlign));
  EXPECT_EQ(memory_pool_capacity(&pool), 3u);
}

TEST_F(MemoryPoolTest, InitOnUnalignedStorageReducesCapacity) {
  memory_pool_t aligned_pool{};
  memory_pool_t bumped_pool{};
  alignas(16) uint8_t big[kStorageSize + 16]{};

  ASSERT_TRUE(memory_pool_init(&aligned_pool, big, kStorageSize, kBlockSize, kBlockAlign));
  EXPECT_EQ(memory_pool_capacity(&aligned_pool), 8u);

  // Same byte count, one byte later: 7 bytes go to the alignment bump, which
  // is enough to cost a whole block.
  ASSERT_TRUE(memory_pool_init(&bumped_pool, big + 1, kStorageSize, kBlockSize, kBlockAlign));
  EXPECT_EQ(memory_pool_capacity(&bumped_pool), 7u);

  // The first block is still properly aligned despite the odd start.
  EXPECT_EQ(reinterpret_cast<uintptr_t>(bumped_pool.base) % kBlockAlign, 0u);
}

TEST_F(MemoryPoolTest, InitClampsBlockAlignToPointerAlignment) {
  // Regression for bug #1: 1, 2 and 4 are legal powers of two, but every free
  // block stores a pointer in its first bytes, so init must clamp up.
  for (size_t align : {size_t{1}, size_t{2}, size_t{4}}) {
    memory_pool_t pool{};
    alignas(16) uint8_t buf[kStorageSize]{};

    ASSERT_TRUE(memory_pool_init(&pool, buf, sizeof buf, kBlockSize, align))
        << "block_align = " << align;
    EXPECT_EQ(pool.block_size, kStride) << "block_align = " << align;

    for (size_t i = 0; i < memory_pool_capacity(&pool); ++i) {
      void *block = memory_pool_alloc(&pool);
      ASSERT_NE(block, nullptr);
      EXPECT_EQ(reinterpret_cast<uintptr_t>(block) % alignof(void *), 0u)
          << "block_align = " << align << ", block " << i;
    }
  }
}

// ---- Alloc --------------------------------------------------------------

TEST_F(MemoryPoolTest, AllocReturnsOwnedAlignedBlocks) {
  for (size_t i = 0; i < kCapacity; ++i) {
    void *block = memory_pool_alloc(&mp);
    ASSERT_NE(block, nullptr) << "alloc " << i;
    EXPECT_TRUE(memory_pool_owns(&mp, block)) << "alloc " << i;
    EXPECT_EQ(reinterpret_cast<uintptr_t>(block) % kBlockAlign, 0u) << "alloc " << i;
  }
}

TEST_F(MemoryPoolTest, AllocReturnsDistinctPointers) {
  auto blocks = AllocAll();
  std::set<void *> unique(blocks.begin(), blocks.end());

  EXPECT_EQ(unique.size(), kCapacity);
  EXPECT_EQ(unique.count(nullptr), 0u);
}

TEST_F(MemoryPoolTest, AllocExhaustsAtCapacityThenReturnsNull) {
  auto blocks = AllocAll();
  for (size_t i = 0; i < kCapacity; ++i) {
    EXPECT_NE(blocks[i], nullptr) << "alloc " << i;
  }

  EXPECT_EQ(memory_pool_used(&mp), kCapacity);
  EXPECT_EQ(memory_pool_available(&mp), 0u);

  // Exhaustion is reported, not fatal, and it changes nothing.
  EXPECT_EQ(memory_pool_alloc(&mp), nullptr);
  EXPECT_EQ(memory_pool_alloc(&mp), nullptr);
  EXPECT_EQ(memory_pool_used(&mp), kCapacity);
  EXPECT_EQ(memory_pool_high_water(&mp), kCapacity);
}

TEST_F(MemoryPoolTest, AllocTracksHighWater) {
  EXPECT_EQ(memory_pool_high_water(&mp), 0u);

  void *a = memory_pool_alloc(&mp);
  void *b = memory_pool_alloc(&mp);
  void *c = memory_pool_alloc(&mp);
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(memory_pool_high_water(&mp), 3u);

  // Freeing lowers used but never the peak.
  ASSERT_TRUE(memory_pool_free(&mp, c));
  ASSERT_TRUE(memory_pool_free(&mp, b));
  EXPECT_EQ(memory_pool_used(&mp), 1u);
  EXPECT_EQ(memory_pool_high_water(&mp), 3u);

  // A genuinely higher peak does raise it.
  for (size_t i = 0; i < kCapacity - 1; ++i) {
    ASSERT_NE(memory_pool_alloc(&mp), nullptr) << "alloc " << i;
  }
  EXPECT_EQ(memory_pool_used(&mp), kCapacity);
  EXPECT_EQ(memory_pool_high_water(&mp), kCapacity);
}

// ---- Free ---------------------------------------------------------------

TEST_F(MemoryPoolTest, FreeThenAllocReturnsSameBlock) {
  void *first = memory_pool_alloc(&mp);
  void *second = memory_pool_alloc(&mp);
  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);

  // LIFO: the block just freed is the next one handed out.
  ASSERT_TRUE(memory_pool_free(&mp, second));
  EXPECT_EQ(memory_pool_alloc(&mp), second);

  ASSERT_TRUE(memory_pool_free(&mp, second));
  ASSERT_TRUE(memory_pool_free(&mp, first));
  EXPECT_EQ(memory_pool_alloc(&mp), first);
}

TEST_F(MemoryPoolTest, FreeOfLastOutstandingBlockSucceeds) {
  void *only = memory_pool_alloc(&mp);
  ASSERT_NE(only, nullptr);
  ASSERT_EQ(memory_pool_used(&mp), 1u);

  // Regression for bug #3: used 1 -> 0 is a VALID free, not an underflow.
  EXPECT_TRUE(memory_pool_free(&mp, only));
  EXPECT_EQ(memory_pool_used(&mp), 0u);
  EXPECT_EQ(memory_pool_available(&mp), kCapacity);
}

TEST_F(MemoryPoolTest, FreeOnEmptyPoolIsRejected) {
  void *block = memory_pool_alloc(&mp);
  ASSERT_NE(block, nullptr);
  ASSERT_TRUE(memory_pool_free(&mp, block));
  ASSERT_EQ(memory_pool_used(&mp), 0u);

  // Regression for bug #3: a genuinely-owned pointer, but nothing is
  // outstanding. This is the one slice of double-free that IS detectable.
  EXPECT_FALSE(memory_pool_free(&mp, block));
  EXPECT_EQ(memory_pool_used(&mp), 0u);  // no wrap to SIZE_MAX
  EXPECT_EQ(memory_pool_available(&mp), kCapacity);
}

TEST_F(MemoryPoolTest, FreeRejectsForeignPointer) {
  void *block = memory_pool_alloc(&mp);
  ASSERT_NE(block, nullptr);

  alignas(16) uint8_t foreign[kStorageSize]{};
  int stack_object = 0;

  EXPECT_FALSE(memory_pool_free(&mp, nullptr));
  EXPECT_FALSE(memory_pool_free(&mp, foreign));
  EXPECT_FALSE(memory_pool_free(&mp, &stack_object));
  EXPECT_EQ(memory_pool_used(&mp), 1u);
}

TEST_F(MemoryPoolTest, RejectedFreeLeavesPoolUnchanged) {
  void *first = memory_pool_alloc(&mp);
  ASSERT_NE(first, nullptr);

  const size_t used_before = memory_pool_used(&mp);
  const size_t avail_before = memory_pool_available(&mp);
  const size_t peak_before = memory_pool_high_water(&mp);

  // Inside the arena, but not a block start.
  uint8_t *mid = static_cast<uint8_t *>(first) + 1;
  EXPECT_FALSE(memory_pool_free(&mp, mid));

  EXPECT_EQ(memory_pool_used(&mp), used_before);
  EXPECT_EQ(memory_pool_available(&mp), avail_before);
  EXPECT_EQ(memory_pool_high_water(&mp), peak_before);

  // The free list is intact: the rest of the pool still hands out blocks.
  for (size_t i = 1; i < kCapacity; ++i) {
    EXPECT_NE(memory_pool_alloc(&mp), nullptr) << "alloc " << i;
  }
  EXPECT_EQ(memory_pool_alloc(&mp), nullptr);
}

// ---- Owns ---------------------------------------------------------------

TEST_F(MemoryPoolTest, OwnsAcceptsEveryBlockStart) {
  auto blocks = AllocAll();
  for (size_t i = 0; i < kCapacity; ++i) {
    ASSERT_NE(blocks[i], nullptr) << "block " << i;
    EXPECT_TRUE(memory_pool_owns(&mp, blocks[i])) << "block " << i;
  }
}

TEST_F(MemoryPoolTest, OwnsRejectsMidBlockPointers) {
  auto blocks = AllocAll();
  for (size_t i = 0; i < kCapacity; ++i) {
    auto *block = static_cast<uint8_t *>(blocks[i]);
    ASSERT_NE(block, nullptr) << "block " << i;

    // Regression for bug #4: being inside the arena is not the same as being
    // a block. A range check alone would accept all three of these.
    EXPECT_FALSE(memory_pool_owns(&mp, block + 1)) << "block " << i << " + 1";
    EXPECT_FALSE(memory_pool_owns(&mp, block + kStride / 2))
        << "block " << i << " + stride/2";
    EXPECT_FALSE(memory_pool_owns(&mp, block + kStride - 1))
        << "block " << i << " + stride-1";
  }
}

TEST_F(MemoryPoolTest, OwnsRejectsOutOfRangePointers) {
  // Start the pool inside a larger buffer so there are legal addresses on
  // both sides of the arena to test with.
  memory_pool_t pool{};
  alignas(16) uint8_t big[kStorageSize + 32]{};
  ASSERT_TRUE(memory_pool_init(&pool, big + 16, kStorageSize, kBlockSize, kBlockAlign));

  uint8_t *base = pool.base;
  uint8_t *end = base + kCapacity * kStride;
  alignas(16) uint8_t foreign[kStorageSize]{};

  EXPECT_TRUE(memory_pool_owns(&pool, base));  // contrast: base itself is fine

  EXPECT_FALSE(memory_pool_owns(&pool, nullptr));
  EXPECT_FALSE(memory_pool_owns(&pool, base - 1));
  EXPECT_FALSE(memory_pool_owns(&pool, base - kStride));
  EXPECT_FALSE(memory_pool_owns(&pool, end));  // one past the last block
  EXPECT_FALSE(memory_pool_owns(&pool, end + 1));
  EXPECT_FALSE(memory_pool_owns(&pool, foreign));
  EXPECT_FALSE(memory_pool_owns(nullptr, base));
}

// ---- Reset --------------------------------------------------------------

TEST_F(MemoryPoolTest, ResetRestoresFullCapacity) {
  AllocAll();
  ASSERT_EQ(memory_pool_used(&mp), kCapacity);
  ASSERT_EQ(memory_pool_alloc(&mp), nullptr);

  memory_pool_reset(&mp);

  EXPECT_EQ(memory_pool_used(&mp), 0u);
  EXPECT_EQ(memory_pool_available(&mp), kCapacity);
  EXPECT_EQ(memory_pool_capacity(&mp), kCapacity);

  // Every block is allocatable again.
  auto again = AllocAll();
  for (size_t i = 0; i < kCapacity; ++i) {
    EXPECT_NE(again[i], nullptr) << "block " << i;
  }
  EXPECT_EQ(memory_pool_alloc(&mp), nullptr);
}

TEST_F(MemoryPoolTest, ResetPreservesHighWater) {
  AllocAll();
  ASSERT_EQ(memory_pool_high_water(&mp), kCapacity);

  memory_pool_reset(&mp);

  // high_water is "peak since init" -- the number you size the pool from.
  EXPECT_EQ(memory_pool_high_water(&mp), kCapacity);
  EXPECT_EQ(memory_pool_used(&mp), 0u);

  // Only a fresh init clears it.
  ASSERT_TRUE(memory_pool_init(&mp, storage, sizeof storage, kBlockSize, kBlockAlign));
  EXPECT_EQ(memory_pool_high_water(&mp), 0u);
}

TEST_F(MemoryPoolTest, ResetOnNullPoolIsNoOp) {
  memory_pool_reset(nullptr);  // must not crash

  // The real pool is untouched by the null call.
  EXPECT_EQ(memory_pool_capacity(&mp), kCapacity);
  EXPECT_EQ(memory_pool_used(&mp), 0u);
}

// ---- Queries ------------------------------------------------------------

TEST_F(MemoryPoolTest, QueriesReturnZeroOnNullPool) {
  EXPECT_EQ(memory_pool_capacity(nullptr), 0u);
  EXPECT_EQ(memory_pool_used(nullptr), 0u);
  EXPECT_EQ(memory_pool_available(nullptr), 0u);
  EXPECT_EQ(memory_pool_high_water(nullptr), 0u);

  // The rest of the API is NULL-safe too.
  EXPECT_EQ(memory_pool_alloc(nullptr), nullptr);
  EXPECT_FALSE(memory_pool_free(nullptr, storage));
  EXPECT_FALSE(memory_pool_owns(nullptr, storage));
}

TEST_F(MemoryPoolTest, UsedPlusAvailableAlwaysEqualsCapacity) {
  std::array<void *, kCapacity> blocks{};

  for (size_t i = 0; i < kCapacity; ++i) {
    blocks[i] = memory_pool_alloc(&mp);
    ASSERT_NE(blocks[i], nullptr) << "alloc " << i;
    EXPECT_EQ(memory_pool_used(&mp) + memory_pool_available(&mp), kCapacity)
        << "after alloc " << i;
  }
  for (size_t i = 0; i < kCapacity; ++i) {
    ASSERT_TRUE(memory_pool_free(&mp, blocks[i])) << "free " << i;
    EXPECT_EQ(memory_pool_used(&mp) + memory_pool_available(&mp), kCapacity)
        << "after free " << i;
  }
}

// ---- Integration --------------------------------------------------------

TEST_F(MemoryPoolTest, BlocksDoNotOverlap) {
  auto blocks = AllocAll();

  // Stamp each block with a distinct byte across its whole requested size.
  for (size_t i = 0; i < kCapacity; ++i) {
    ASSERT_NE(blocks[i], nullptr) << "block " << i;
    std::memset(blocks[i], static_cast<int>(0xA0u + i), kBlockSize);
  }

  // Re-read every byte: no block was clobbered by a neighbour.
  for (size_t i = 0; i < kCapacity; ++i) {
    const auto *bytes = static_cast<const uint8_t *>(blocks[i]);
    for (size_t j = 0; j < kBlockSize; ++j) {
      ASSERT_EQ(bytes[j], static_cast<uint8_t>(0xA0u + i))
          << "block " << i << ", byte " << j;
    }
  }
}

TEST_F(MemoryPoolTest, ChurnPreservesInvariants) {
  std::vector<void *> live;
  uint32_t rng = 12345u;  // fixed seed, so a failure reproduces exactly

  for (int op = 0; op < 20000; ++op) {
    const bool do_alloc = live.empty() || (NextRandom(rng) & 1u) != 0u;

    if (do_alloc) {
      void *block = memory_pool_alloc(&mp);
      if (block != nullptr) {
        ASSERT_TRUE(memory_pool_owns(&mp, block)) << "op " << op;
        live.push_back(block);
      } else {
        ASSERT_EQ(live.size(), kCapacity) << "op " << op;  // only NULL when full
      }
    } else {
      const size_t idx = static_cast<size_t>(NextRandom(rng)) % live.size();
      ASSERT_TRUE(memory_pool_free(&mp, live[idx])) << "op " << op;
      live[idx] = live.back();
      live.pop_back();
    }

    ASSERT_EQ(memory_pool_used(&mp), live.size()) << "op " << op;
    ASSERT_EQ(memory_pool_used(&mp) + memory_pool_available(&mp), kCapacity)
        << "op " << op;
  }

  // The pool survived: reset and it is whole again.
  memory_pool_reset(&mp);
  EXPECT_EQ(memory_pool_available(&mp), kCapacity);
  auto blocks = AllocAll();
  for (size_t i = 0; i < kCapacity; ++i) {
    EXPECT_NE(blocks[i], nullptr) << "block " << i;
  }
}

} // namespace
