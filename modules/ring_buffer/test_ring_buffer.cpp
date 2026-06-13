#include <cstdint>
#include <gtest/gtest.h>

extern "C" {
#include "ring_buffer.h"
}

namespace {

class RingBufferTest : public ::testing::Test {
  protected:
    static constexpr size_t kCapacity = 8;
    uint8_t storage[kCapacity]{};
    ring_buffer_t rb{};

    void SetUp() override {
      ASSERT_TRUE(ring_buffer_init(&rb, storage, kCapacity));
    }
};

TEST_F(RingBufferTest, InitRejectsNonPowerOfTwo) {
  ring_buffer_t bad{};
  uint8_t buf[6]{};
  EXPECT_FALSE(ring_buffer_init(&bad, buf, 6));
}

TEST_F(RingBufferTest, InitRejectsNullAndZero){
  ring_buffer_t bad{};
  uint8_t buf[8]{};
  EXPECT_FALSE(ring_buffer_init(nullptr, buf, 8));
  EXPECT_FALSE(ring_buffer_init(&bad, nullptr, 8));
  EXPECT_FALSE(ring_buffer_init(&bad, buf, 0));
}

TEST_F(RingBufferTest, StartsEmpty) {
  EXPECT_TRUE(ring_buffer_is_empty(&rb));
  EXPECT_FALSE(ring_buffer_is_full(&rb));
  EXPECT_EQ(ring_buffer_count(&rb), 0u);
  EXPECT_EQ(ring_buffer_free(&rb), kCapacity);
}

TEST_F(RingBufferTest, PutThenGetRoundTrips) {
  EXPECT_TRUE(ring_buffer_put(&rb, 0x42));
  EXPECT_EQ(ring_buffer_count(&rb), 1u);
  uint8_t out = 0;
  EXPECT_TRUE(ring_buffer_get(&rb, &out));
  EXPECT_EQ(out, 0x42);
  EXPECT_TRUE(ring_buffer_is_empty(&rb));
}

TEST_F(RingBufferTest, FillsToCapacityThenRejects) {
  for (size_t i = 0; i < kCapacity; ++i) {
    EXPECT_TRUE(ring_buffer_put(&rb, static_cast<uint8_t>(i)));
  }
  EXPECT_TRUE(ring_buffer_is_full(&rb));
  EXPECT_FALSE(ring_buffer_put(&rb, 0xFF));
  EXPECT_EQ(ring_buffer_count(&rb), kCapacity);
}

TEST_F(RingBufferTest, GetOnEmptyReturnsFalse) {
  uint8_t out = 0xAB;
  EXPECT_FALSE(ring_buffer_get(&rb, &out));
  EXPECT_EQ(out, 0xAB);
}

TEST_F(RingBufferTest, FifoOrderPreserved) {
  const uint8_t in[] = {1, 2, 3, 4, 5};
  EXPECT_EQ(ring_buffer_write(&rb, in, sizeof(in)), sizeof(in));
  uint8_t out[sizeof(in)]{};
  EXPECT_EQ(ring_buffer_read(&rb, out, sizeof(in)), sizeof(in));
  EXPECT_EQ(0, memcmp(in, out, sizeof(in)));
}

TEST_F(RingBufferTest, WrapsAroundCorrectly) {
    // Fill, drain half, refill past the wrap point.
    for (uint8_t i = 0; i < kCapacity; ++i) ring_buffer_put(&rb, i);
    uint8_t out = 0;
    for (int i = 0; i < 4; ++i) ring_buffer_get(&rb, &out);
    for (uint8_t i = 100; i < 104; ++i) EXPECT_TRUE(ring_buffer_put(&rb, i));

    // Expected remaining sequence: 4,5,6,7,100,101,102,103
    const uint8_t expected[] = {4, 5, 6, 7, 100, 101, 102, 103};
    for (uint8_t e : expected) {
        ASSERT_TRUE(ring_buffer_get(&rb, &out));
        EXPECT_EQ(out, e);
    }
    EXPECT_TRUE(ring_buffer_is_empty(&rb));
}

TEST_F(RingBufferTest, WriteStopsWhenFull) {
    const uint8_t in[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_EQ(ring_buffer_write(&rb, in, sizeof(in)), kCapacity);
    EXPECT_TRUE(ring_buffer_is_full(&rb));
}

TEST_F(RingBufferTest, ResetClears) {
    ring_buffer_put(&rb, 1);
    ring_buffer_put(&rb, 2);
    ring_buffer_reset(&rb);
    EXPECT_TRUE(ring_buffer_is_empty(&rb));
    EXPECT_EQ(ring_buffer_count(&rb), 0u);
}

}  // namespace
