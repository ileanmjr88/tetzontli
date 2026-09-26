#include "state_machine.h"

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
