#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <iterator>

extern "C" {
#include "state_machine.h"
}

namespace {

// Simulated robot joint motor controller.

enum : sm_state_t { IDLE, ARMED, RUNNING, FAULT, STATE_COUNT };

enum : sm_event_t {
  EV_ARM,
  EV_DISARM,
  EV_START,
  EV_STOP,
  EV_SET_SPEED,
  EV_OVERCURRENT,
  EV_ESTOP,
  EV_RESET,
};

// Guards and actions are nullptr until the sm_start / sm_dispatch slices
// add motor_t; the comment on each row says what goes there.
constexpr sm_transition_t kMotorTable[] = {
    // from          event           guard    action   to
    {SM_ANY_STATE, EV_ESTOP, nullptr, nullptr, FAULT}, // action: motor_off
    {IDLE, EV_ARM, nullptr, nullptr, ARMED},
    {ARMED, EV_DISARM, nullptr, nullptr, IDLE},
    {ARMED, EV_START, nullptr, nullptr, RUNNING}, // guard: battery_ok
    {ARMED, EV_START, nullptr, nullptr, ARMED},   // action: warn_battery_low
    {RUNNING, EV_STOP, nullptr, nullptr, ARMED},
    {RUNNING, EV_SET_SPEED, nullptr, nullptr, RUNNING}, // action: set_speed
    {RUNNING, EV_OVERCURRENT, nullptr, nullptr, FAULT}, // action: motor_off
    {FAULT, EV_RESET, nullptr, nullptr, IDLE},          // guard: fault_cleared
};

constexpr size_t kMotorRows = std::size(kMotorTable);

class StateMachineTest : public ::testing::Test {
protected:
  sm_t sm{};
  void SetUp() override {
    ASSERT_TRUE(sm_init(&sm, kMotorTable, kMotorRows, nullptr, STATE_COUNT,
                        IDLE, nullptr));
  }
};

TEST(StateMachineInit, RejectsNullTable) {
  sm_t sm;
  EXPECT_FALSE(
      sm_init(&sm, nullptr, kMotorRows, nullptr, STATE_COUNT, IDLE, nullptr));
}

TEST(StateMachineInit, RejectsNullMachine) {
  EXPECT_FALSE(sm_init(nullptr, kMotorTable, kMotorRows, nullptr, STATE_COUNT,
                       IDLE, nullptr));
}

TEST(StateMachineInit, RejectsZeroCount) {
  sm_t sm;
  EXPECT_FALSE(
      sm_init(&sm, kMotorTable, 0, nullptr, STATE_COUNT, IDLE, nullptr));
}

TEST(StateMachineInit, RejectsZeroStateCount) {
  sm_t sm;
  EXPECT_FALSE(
      sm_init(&sm, kMotorTable, kMotorRows, nullptr, 0, IDLE, nullptr));
}

TEST(StateMachineInit, RejectsInitialOutOfRange) {
  sm_t sm;
  EXPECT_FALSE(sm_init(&sm, kMotorTable, kMotorRows, nullptr, STATE_COUNT,
                       STATE_COUNT, nullptr));
}

TEST(StateMachineInit, RejectsRowWithBadFrom) {
  sm_t sm;
  const sm_transition_t table[] = {
      {IDLE, EV_ARM, nullptr, nullptr, ARMED},
      {STATE_COUNT, EV_ARM, nullptr, nullptr, IDLE}};
  const size_t tableSize = std::size(table);
  EXPECT_FALSE(
      sm_init(&sm, table, tableSize, nullptr, STATE_COUNT, IDLE, nullptr));
}

TEST(StateMachineInit, RejectsRowWithBadTo) {
  sm_t sm;
  const sm_transition_t table[] = {
      {IDLE, EV_ARM, nullptr, nullptr, ARMED},
      {ARMED, EV_ARM, nullptr, nullptr, STATE_COUNT}};
  const size_t tableSize = std::size(table);
  EXPECT_FALSE(
      sm_init(&sm, table, tableSize, nullptr, STATE_COUNT, IDLE, nullptr));
}

TEST(StateMachineInit, RejectsRowWithNoEventEvent) {
  sm_t sm;
  const sm_transition_t table[] = {
      {IDLE, EV_ARM, nullptr, nullptr, ARMED},
      {ARMED, SM_NO_EVENT, nullptr, nullptr, IDLE}};
  const size_t tableSize = std::size(table);
  EXPECT_FALSE(
      sm_init(&sm, table, tableSize, nullptr, STATE_COUNT, IDLE, nullptr));
}

TEST(StateMachineInit, RejectsRowWithAnyStateInTo) {
  sm_t sm;
  const sm_transition_t table[] = {
      {IDLE, EV_ARM, nullptr, nullptr, ARMED},
      {ARMED, EV_ARM, nullptr, nullptr, SM_ANY_STATE}};
  const size_t tableSize = std::size(table);
  EXPECT_FALSE(
      sm_init(&sm, table, tableSize, nullptr, STATE_COUNT, IDLE, nullptr));
}

TEST(StateMachineInit, AcceptsAnyStateInFrom) {
  sm_t sm;
  const sm_transition_t table[] = {
      {IDLE, EV_ARM, nullptr, nullptr, ARMED},
      {SM_ANY_STATE, EV_ESTOP, nullptr, nullptr, FAULT}};
  const size_t tableSize = std::size(table);
  EXPECT_TRUE(
      sm_init(&sm, table, tableSize, nullptr, STATE_COUNT, IDLE, nullptr));
}

TEST(StateMachineInit, FailedInitZeroesMachine) {
  sm_t sm;
  std::memset(&sm, 0xAB, sizeof sm);
  EXPECT_FALSE(
      sm_init(&sm, nullptr, kMotorRows, nullptr, STATE_COUNT, IDLE, nullptr));
  EXPECT_EQ(sm.table, nullptr);
  EXPECT_FALSE(sm.started);
}

TEST(StateMachineInit, StateOfNullIsAnyState) {
  sm_t sm;
  EXPECT_TRUE(sm_init(&sm, kMotorTable, kMotorRows, nullptr, STATE_COUNT, IDLE,
                      nullptr));
  EXPECT_EQ(sm_state(nullptr), SM_ANY_STATE);
}

TEST_F(StateMachineTest, StartsInInitialState) {
  EXPECT_EQ(sm_state(&sm), IDLE);
}

TEST_F(StateMachineTest, IsNotStartedAfterInit) { EXPECT_FALSE(sm.started); }

TEST_F(StateMachineTest, AcceptsNullHooks) { EXPECT_EQ(sm.hooks, nullptr); }

} // namespace
