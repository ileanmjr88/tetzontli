#include <cstring>
#include <gtest/gtest.h>
#include <iterator>
#include <string>

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

typedef struct {
  bool brake_engaged;
  bool pwm_enabled;
  bool battery_ok;
  bool fault_cleared;
  std::string log;
} motor_t;

void idle_entry(void *ctx) {
  auto *m = static_cast<motor_t *>(ctx);
  m->brake_engaged = true;
  m->log += 'I';
}

void running_entry(void *ctx) {
  auto *m = static_cast<motor_t *>(ctx);
  m->pwm_enabled = true;
  m->log += 'R';
}

void running_exit(void *ctx) {
  auto *m = static_cast<motor_t *>(ctx);
  m->pwm_enabled = false;
  m->log += 'r';
}

void fault_entry(void *ctx) {
  auto *m = static_cast<motor_t *>(ctx);
  m->brake_engaged = true;
  m->pwm_enabled = false;
  m->log += 'F';
}

bool battery_ok(void *ctx) {
  auto *m = static_cast<motor_t *>(ctx);
  return m->battery_ok;
}

bool fault_cleared(void *ctx) {
  auto *m = static_cast<motor_t *>(ctx);
  return m->fault_cleared;
}

void motor_off(void *ctx) {
  auto *m = static_cast<motor_t *>(ctx);
  m->pwm_enabled = false;
  m->log += 'm';
}

void warn_battery_low(void *ctx) {
  auto *m = static_cast<motor_t *>(ctx);
  m->log += 'w';
}

void set_speed(void *ctx) {
  auto *m = static_cast<motor_t *>(ctx);
  m->log += 's';
}

// Safety row first: first match wins, so nothing can shadow the e-stop.
constexpr sm_transition_t kMotorTable[] = {
    // from          event           guard          action            to
    {SM_ANY_STATE, EV_ESTOP, nullptr, motor_off, FAULT},
    {IDLE, EV_ARM, nullptr, nullptr, ARMED},
    {ARMED, EV_DISARM, nullptr, nullptr, IDLE},
    {ARMED, EV_START, battery_ok, nullptr, RUNNING},
    {ARMED, EV_START, nullptr, warn_battery_low, ARMED},
    {RUNNING, EV_STOP, nullptr, nullptr, ARMED},
    {RUNNING, EV_SET_SPEED, nullptr, set_speed, RUNNING},
    {RUNNING, EV_OVERCURRENT, nullptr, motor_off, FAULT},
    {FAULT, EV_RESET, fault_cleared, nullptr, IDLE},
};

constexpr size_t kMotorRows = std::size(kMotorTable);

constexpr sm_state_hooks_t kMotorHooks[STATE_COUNT] = {
    {idle_entry, nullptr, nullptr},
    {},
    {running_entry, running_exit, nullptr},
    {fault_entry, nullptr, nullptr},
};

class StateMachineTest : public ::testing::Test {
protected:
  motor_t motor{};
  sm_t sm{};

  void SetUp() override {
    ASSERT_TRUE(sm_init(&sm, kMotorTable, kMotorRows, kMotorHooks, STATE_COUNT,
                        IDLE, &motor));
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

TEST(StateMachineInit, AcceptsNullHooks) {
  sm_t sm;
  EXPECT_TRUE(sm_init(&sm, kMotorTable, kMotorRows, nullptr, STATE_COUNT, IDLE,
                      nullptr));
  EXPECT_EQ(sm.hooks, nullptr);
}

TEST_F(StateMachineTest, StartsInInitialState) {
  EXPECT_EQ(sm_state(&sm), IDLE);
}

TEST_F(StateMachineTest, IsNotStartedAfterInit) { EXPECT_FALSE(sm.started); }

TEST_F(StateMachineTest, StartRunsInitialEntryOnce) {
  EXPECT_TRUE(sm_start(&sm));
  EXPECT_TRUE(motor.brake_engaged);
  EXPECT_EQ(motor.log, "I");
}

TEST_F(StateMachineTest, StartSetsStarted) {
  EXPECT_TRUE(sm_start(&sm));
  EXPECT_TRUE(sm.started);
}

TEST_F(StateMachineTest, SecondStartIsRejected) {
  EXPECT_TRUE(sm_start(&sm));
  EXPECT_FALSE(sm_start(&sm));
  EXPECT_EQ(motor.log, "I");
}

TEST(StateMachineStart, StartAfterFailedInitIsRejected) {
  motor_t motor{};
  sm_t sm;
  EXPECT_FALSE(sm_init(&sm, nullptr, kMotorRows, kMotorHooks, STATE_COUNT, IDLE,
                       &motor));
  EXPECT_FALSE(sm_start(&sm));
  EXPECT_EQ(motor.log, "");
}

TEST(StateMachineStart, StartWithoutHooksSucceeds) {
  motor_t motor{};
  sm_t sm;
  EXPECT_TRUE(sm_init(&sm, kMotorTable, kMotorRows, nullptr, STATE_COUNT, IDLE,
                      &motor));
  EXPECT_TRUE(sm_start(&sm));
}

TEST(StateMachineStart, StartNullIsRejected) {
  EXPECT_FALSE(sm_start(nullptr));
}

class StartedStateMachineTest : public StateMachineTest {
protected:
  void SetUp() override {
    StateMachineTest::SetUp();
    ASSERT_TRUE(sm_start(&sm));
    motor.log.clear();
  }

  void GoToRunning() {
    motor.battery_ok = true;
    ASSERT_EQ(sm_dispatch(&sm, EV_ARM), SM_HANDLED);
    ASSERT_EQ(sm_dispatch(&sm, EV_START), SM_HANDLED);
    motor.log.clear();
  }
};

TEST(StateMachineDispatch, DispatchNullIsError) {
  EXPECT_EQ(sm_dispatch(nullptr, EV_ARM), SM_ERROR);
}

TEST_F(StateMachineTest, DispatchBeforeStartIsError) {
  EXPECT_EQ(sm_dispatch(&sm, EV_ARM), SM_ERROR);
  EXPECT_EQ(motor.log, "");
}

TEST_F(StartedStateMachineTest, DispatchCorruptedStateIsError) {
  sm.current = STATE_COUNT;
  EXPECT_EQ(sm_dispatch(&sm, EV_ARM), SM_ERROR);
  EXPECT_EQ(motor.log, "");
}

TEST_F(StartedStateMachineTest, DispatchMovesToTargetState) {
  EXPECT_EQ(sm_dispatch(&sm, EV_ARM), SM_HANDLED);
  EXPECT_EQ(sm_state(&sm), ARMED);
  EXPECT_EQ(motor.log, "");
}

TEST_F(StartedStateMachineTest, UnhandledEventLeavesStateUnchanged) {
  EXPECT_EQ(sm_dispatch(&sm, EV_STOP), SM_UNHANDLED);
  EXPECT_EQ(sm_state(&sm), IDLE);
  EXPECT_EQ(motor.log, "");
}

TEST_F(StartedStateMachineTest, NoEventIsUnhandled) {
  EXPECT_EQ(sm_dispatch(&sm, SM_NO_EVENT), SM_UNHANDLED);
  EXPECT_EQ(sm_state(&sm), IDLE);
}

TEST_F(StartedStateMachineTest, TransitionRunsExitActionEntryInOrder) {
  ASSERT_NO_FATAL_FAILURE(GoToRunning());
  EXPECT_EQ(sm_dispatch(&sm, EV_OVERCURRENT), SM_HANDLED);
  EXPECT_EQ(sm_state(&sm), FAULT);
  EXPECT_EQ(motor.log, "rmF");
  EXPECT_FALSE(motor.pwm_enabled);
}

TEST_F(StartedStateMachineTest, SelfTransitionRunsOnlyAction) {
  ASSERT_NO_FATAL_FAILURE(GoToRunning());
  EXPECT_EQ(sm_dispatch(&sm, EV_SET_SPEED), SM_HANDLED);
  EXPECT_EQ(sm_state(&sm), RUNNING);
  EXPECT_EQ(motor.log, "s");
}

TEST_F(StartedStateMachineTest, GuardPassFiresRow) {
  motor.battery_ok = true;
  ASSERT_EQ(sm_dispatch(&sm, EV_ARM), SM_HANDLED);
  motor.log.clear();
  EXPECT_EQ(sm_dispatch(&sm, EV_START), SM_HANDLED);
  EXPECT_EQ(sm_state(&sm), RUNNING);
  EXPECT_EQ(motor.log, "R");
}

TEST_F(StartedStateMachineTest, GuardRejectFallsThroughToNextRow) {
  motor.battery_ok = false;
  ASSERT_EQ(sm_dispatch(&sm, EV_ARM), SM_HANDLED);
  motor.log.clear();
  EXPECT_EQ(sm_dispatch(&sm, EV_START), SM_HANDLED);
  EXPECT_EQ(sm_state(&sm), ARMED);
  EXPECT_EQ(motor.log, "w");
}

TEST_F(StartedStateMachineTest, AllGuardsRejectedReturnsGuardRejected) {
  ASSERT_EQ(sm_dispatch(&sm, EV_ESTOP), SM_HANDLED);
  motor.log.clear();
  motor.fault_cleared = false;

  EXPECT_EQ(sm_dispatch(&sm, EV_RESET), SM_GUARD_REJECTED);
  EXPECT_EQ(sm_state(&sm), FAULT);
  EXPECT_EQ(motor.log, "");
}

TEST_F(StartedStateMachineTest, EstopReachesFaultFromEveryState) {
  // idle_entry already engaged the brake, so clear it before each e-stop or
  // the brake check proves nothing.
  auto estop_from = [this](sm_state_t from, const char *expected_log) {
    ASSERT_EQ(sm_state(&sm), from);
    motor.brake_engaged = false;
    motor.log.clear();
    EXPECT_EQ(sm_dispatch(&sm, EV_ESTOP), SM_HANDLED);
    EXPECT_EQ(sm_state(&sm), FAULT);
    EXPECT_TRUE(motor.brake_engaged);
    EXPECT_FALSE(motor.pwm_enabled);
    EXPECT_EQ(motor.log, expected_log);
  };
  motor.fault_cleared = true;

  ASSERT_NO_FATAL_FAILURE(estop_from(IDLE, "mF"));
  ASSERT_EQ(sm_dispatch(&sm, EV_RESET), SM_HANDLED);

  ASSERT_EQ(sm_dispatch(&sm, EV_ARM), SM_HANDLED);
  ASSERT_NO_FATAL_FAILURE(estop_from(ARMED, "mF"));
  ASSERT_EQ(sm_dispatch(&sm, EV_RESET), SM_HANDLED);

  ASSERT_NO_FATAL_FAILURE(GoToRunning());
  ASSERT_NO_FATAL_FAILURE(estop_from(RUNNING, "rmF"));
}

TEST_F(StartedStateMachineTest, EstopInFaultIsSelfTransition) {
  ASSERT_EQ(sm_dispatch(&sm, EV_ESTOP), SM_HANDLED);
  motor.log.clear();

  EXPECT_EQ(sm_dispatch(&sm, EV_ESTOP), SM_HANDLED);
  EXPECT_EQ(sm_state(&sm), FAULT);
  EXPECT_EQ(motor.log, "m");
}

TEST_F(StartedStateMachineTest, FullCycle) {
  motor.battery_ok = true;
  motor.fault_cleared = true;

  ASSERT_EQ(sm_dispatch(&sm, EV_ARM), SM_HANDLED);
  ASSERT_EQ(sm_state(&sm), ARMED);
  ASSERT_EQ(sm_dispatch(&sm, EV_START), SM_HANDLED);
  ASSERT_EQ(sm_state(&sm), RUNNING);
  EXPECT_TRUE(motor.pwm_enabled);
  ASSERT_EQ(sm_dispatch(&sm, EV_OVERCURRENT), SM_HANDLED);
  ASSERT_EQ(sm_state(&sm), FAULT);
  EXPECT_FALSE(motor.pwm_enabled);
  ASSERT_EQ(sm_dispatch(&sm, EV_RESET), SM_HANDLED);
  ASSERT_EQ(sm_state(&sm), IDLE);

  EXPECT_TRUE(motor.brake_engaged);
  EXPECT_EQ(motor.log, "RrmFI");
}

} // namespace
