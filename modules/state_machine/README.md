# Finite State Machine (Table-Driven)

**English** · [Español](README.es.md)

A table-driven finite state machine over caller-supplied const tables.

**[Blog](https://www.ilean.me/blog/common-embedded-patterns-finite-state-machine/)**

## Why it matters

Almost every embedded system is a state machine: a debouncer, a modem driver, a
battery charger, a bootloader. Most start life as `switch(state)` with a nested
`switch(event)` in each case. That works for three states. By ten, the behavior
is scattered across hundreds of lines, entry and exit code is copy-pasted into
every case that touches a state, rules that apply to every state have no
natural home, and a missing `break` silently falls through into the next state.

A transition table puts the whole behavior on one screen. Each row is one rule —
*in `from`, on `event`, if `guard` passes, run `action` and go to `to`* — so the
table reads like a specification, and "what happens if X arrives in state Y?"
is answered by reading rows instead of tracing branches.

## Design decisions

- **Behavior is data.** Rules live in a `const sm_transition_t[]`, so the table
  can sit in flash. Per-state `on_entry` / `on_exit` / `on_run` hooks live in
  an optional second table, so entry and exit code is written once per state.
- **First match wins.** Rows are tried top to bottom. `SM_ANY_STATE` safety rows
  (an e-stop) go first; a guarded row goes above its unguarded fallback. Row
  order is behavior.
- **Guards are questions.** A failed guard keeps scanning, which is how
  fallbacks work. Guards may run for rows that never fire, so they must be
  side-effect free.
- **Exit, action, entry.** The old state cleans up, the transition does its
  work, then the new state sets itself up.
- **Self-transitions run only the action.** Changing a setpoint in `RUNNING`
  shouldn't stop and restart the motor; in embedded code, entry and exit
  usually touch hardware.
- **`on_run` returns an event instead of dispatching it.** A state can poll the
  world (a sensor, a timeout, an ISR flag) and report what happened; the
  machine dispatches after the hook returns, so it never re-enters itself.
- **Four results, not a `bool`.** `SM_UNHANDLED` (no rule exists) is separate
  from `SM_GUARD_REJECTED` (rules exist, every guard said no), and both are
  separate from `SM_ERROR` (misuse).
- **Validate once at init.** `sm_init` checks every row up front and zeroes the
  machine before validating, so a failed init leaves a machine that
  `sm_start`, `sm_dispatch` and `sm_run` all refuse. `sm_init` runs no user
  code; `sm_start` enters the initial state.
- **`uint8_t` states and events.** Rows stay small, at the cost of 255 of each;
  `0xFF` is reserved for `SM_ANY_STATE` and `SM_NO_EVENT`.
- **`void *ctx` everywhere.** Every callback gets the caller's context, so there
  are no globals, several machines can run side by side, and tests can hand in
  a fake.
- **Not ISR-safe, not re-entrant.** Guards, actions and hooks must not call
  `sm_dispatch` or `sm_run` on the same machine. Queue events from an ISR (a
  `ring_buffer` carries one-byte events as is) and dispatch them from one
  context.
- **Flat, linear scan.** No hierarchical states; behavior shared across states
  goes in `SM_ANY_STATE` rows. Each dispatch walks the table, which is fine for
  dozens of rows.

## API

See [`state_machine.h`](state_machine.h). Core operations: `init`, `start`,
`dispatch`, `run`, plus `state`. Declare the hooks table as
`hooks[STATE_COUNT]`, with `STATE_COUNT` as the last value of your state
`enum`: `sm_init` only gets a pointer and can't check its length, but the
compiler flags extra entries and zero-fills missing ones to `NULL` (no hook).
The test suite wires up a full motor controller and doubles as the usage
example.
