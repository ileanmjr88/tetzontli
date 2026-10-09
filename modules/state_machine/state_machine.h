/**
 * @file state_machine.h
 * @brief Table-driven finite state machine over caller-supplied const tables.
 *
 * Behaviour is data: each transition row says "in `from`, on `event`, if
 * `guard` passes, run `action` and go to `to`". An optional per-state hooks
 * table adds on_entry / on_exit / on_run. Both tables are caller-owned and
 * can live in flash; nothing is allocated. See README.md for the rationale.
 *
 * Contract:
 *
 * - Not ISR-safe or thread-safe.
 * - No re-entrant calls: guards, actions and hooks must not call sm_dispatch()
 *   or sm_run() on the same machine. Return the event from on_run instead.
 * - Rows are tried top to bottom; the first match whose guard passes wins. Put
 *   SM_ANY_STATE safety rows first.
 * - Guards must be side-effect free; they may run for rows that don't fire.
 * - Transition order: on_exit(old), action, state = to, on_entry(new).
 *   A self-transition (to == current) runs only the action.
 * - sm_init() runs no user code. Call sm_start() before dispatching;
 *   until then sm_dispatch() and sm_run() return SM_ERROR.
 * - Every function is NULL-safe, and every hook is optional.
 */

#ifndef TETZONTLI_STATE_MACHINE_H
#define TETZONTLI_STATE_MACHINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t sm_state_t; /**< 0 .. state_count - 1; 0xFF reserved. */
typedef uint8_t sm_event_t; /**< Caller-defined; 0xFF reserved.       */

#define SM_ANY_STATE ((sm_state_t)0xFFu) /**< `from` wildcard: any state.  */
#define SM_NO_EVENT ((sm_event_t)0xFFu)  /**< on_run: nothing to dispatch. */

typedef bool (*sm_guard_fn)(void *ctx);     /**< true = allow the row.  */
typedef void (*sm_action_fn)(void *ctx);    /**< Action, entry or exit. */
typedef sm_event_t (*sm_run_fn)(void *ctx); /**< Event or SM_NO_EVENT.  */

/** One transition table row. */
typedef struct {
  sm_state_t from;     /**< State, or SM_ANY_STATE.  */
  sm_event_t event;    /**< Triggering event.        */
  sm_guard_fn guard;   /**< NULL = always passes.    */
  sm_action_fn action; /**< NULL = no action.        */
  sm_state_t to;       /**< Next state.              */
} sm_transition_t;

/** Per-state hooks; the table has state_count entries. */
typedef struct {
  sm_action_fn on_entry; /**< NULL = none. */
  sm_action_fn on_exit;  /**< NULL = none. */
  sm_run_fn on_run;      /**< NULL = none. */
} sm_state_hooks_t;

/** Machine handle. Fields are readable but must not be written. */
typedef struct {
  const sm_transition_t *table;
  size_t count;
  const sm_state_hooks_t *hooks;
  sm_state_t state_count;
  sm_state_t current;
  bool started;
  void *ctx;
} sm_t;

typedef enum {
  SM_HANDLED,        /**< A row fired, or sm_run() had nothing to do. */
  SM_UNHANDLED,      /**< No row matches.                             */
  SM_GUARD_REJECTED, /**< Rows matched; every guard rejected.         */
  SM_ERROR           /**< NULL, not started, or state out of range.   */
} sm_result_t;

/**
 * @brief Validate the tables and place the machine in @p initial.
 *
 * @param hooks state_count entries, or NULL for no hooks.
 * @param ctx   Passed to every guard, action and hook; may be NULL.
 * @return false if a pointer is NULL, @p count or @p state_count is 0,
 *         @p initial is out of range, or a row has an out-of-range `from` /
 *         `to` or an SM_NO_EVENT event. On failure *sm is zeroed.
 */
bool sm_init(sm_t *sm, const sm_transition_t *table, size_t count,
             const sm_state_hooks_t *hooks, sm_state_t state_count,
             sm_state_t initial, void *ctx);

/**
 * @brief Run the initial state's on_entry and make the machine live.
 * @return false if @p sm is NULL, uninitialized, or already started.
 */
bool sm_start(sm_t *sm);

/**
 * @brief Deliver @p event; fires the first matching row whose guard passes.
 * @return See sm_result_t. The state is unchanged unless SM_HANDLED.
 */
sm_result_t sm_dispatch(sm_t *sm, sm_event_t event);

/**
 * @brief Call the current state's on_run, then dispatch any event it returns.
 * @return The dispatch result, SM_HANDLED if nothing to dispatch, or SM_ERROR.
 */
sm_result_t sm_run(sm_t *sm);

/** @return Current state, or SM_ANY_STATE if @p sm is NULL. */
sm_state_t sm_state(const sm_t *sm);

#ifdef __cplusplus
}
#endif

#endif // TETZONTLI_STATE_MACHINE_H
