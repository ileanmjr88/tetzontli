#include "state_machine.h"

static void run_entry(const sm_t *sm, sm_state_t state) {
  if (sm->hooks != NULL && sm->hooks[state].on_entry != NULL) {
    sm->hooks[state].on_entry(sm->ctx);
  }
}

static void run_exit(const sm_t *sm, sm_state_t state) {
  if (sm->hooks != NULL && sm->hooks[state].on_exit != NULL) {
    sm->hooks[state].on_exit(sm->ctx);
  }
}

bool sm_init(sm_t *sm, const sm_transition_t *table, size_t count,
             const sm_state_hooks_t *hooks, sm_state_t state_count,
             sm_state_t initial, void *ctx) {
  if (sm == NULL) {
    return false;
  }

  *sm = (sm_t){0};

  if (table == NULL || count == 0u || state_count == 0u ||
      initial >= state_count) {
    return false;
  }

  for (size_t i = 0u; i < count; i++) {
    if (table[i].from != SM_ANY_STATE && table[i].from >= state_count) {
      return false;
    }
    if (table[i].to >= state_count || table[i].event == SM_NO_EVENT) {
      return false;
    }
  }

  sm->table = table;
  sm->count = count;
  sm->hooks = hooks;
  sm->state_count = state_count;
  sm->ctx = ctx;
  sm->current = initial;

  return true;
}

sm_state_t sm_state(const sm_t *sm) {
  return (sm == NULL) ? SM_ANY_STATE : sm->current;
}

bool sm_start(sm_t *sm) {
  if (sm == NULL || sm->table == NULL || sm->started) {
    return false;
  }

  run_entry(sm, sm->current);

  sm->started = true;
  return true;
}

sm_result_t sm_dispatch(sm_t *sm, sm_event_t event) {
  if (sm == NULL || !sm->started || sm->current >= sm->state_count) {
    return SM_ERROR;
  }

  bool matched = false;
  for (size_t i = 0u; i < sm->count; i++) {
    const sm_transition_t *row = &sm->table[i];
    if ((row->from != sm->current && row->from != SM_ANY_STATE) ||
        row->event != event) {
      continue;
    }

    matched = true;
    if (row->guard != NULL && !row->guard(sm->ctx)) {
      continue;
    }

    if (row->to != sm->current) {
      run_exit(sm, sm->current);
    }
    if (row->action != NULL) {
      row->action(sm->ctx);
    }
    if (row->to != sm->current) {
      sm->current = row->to;
      run_entry(sm, sm->current);
    }
    return SM_HANDLED;
  }
  return matched ? SM_GUARD_REJECTED : SM_UNHANDLED;
}

sm_result_t sm_run(sm_t *sm) {
  if (sm == NULL || !sm->started || sm->current >= sm->state_count) {
    return SM_ERROR;
  }

  if (sm->hooks == NULL || sm->hooks[sm->current].on_run == NULL) {
    return SM_HANDLED;
  }

  sm_event_t ev = sm->hooks[sm->current].on_run(sm->ctx);
  if (ev == SM_NO_EVENT) {
    return SM_HANDLED;
  }
  return sm_dispatch(sm, ev);
}
