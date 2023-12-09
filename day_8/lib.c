#include <cfac/darray.h>
#include <cfac/hashtable.h>
#include <cfac/log.h>

#include "common.h"
#include "lib.h"

#include <string.h>

static STAT_Val init_machine(StateMachine * machine) {
  CHECK(machine != NULL);

  *machine = (StateMachine){0};

  TRY(DAR_create(&machine->states, sizeof(State)));

  return OK;
}

static STAT_Val parse_state_name(SPN_Span line, SPN_MutSpan name) {
  CHECK(!SPN_is_empty(line));
  CHECK(line.element_size == sizeof(char));
  CHECK(line.len >= 3);
  CHECK(!SPN_is_empty(SPN_mut_to_const(name)));
  CHECK(name.element_size == sizeof(char));
  CHECK(name.len >= 4);

  const SPN_Span name_span = SPN_subspan(line, 0, 3);

  memcpy(name.begin, name_span.begin, name_span.len);
  ((char *)name.begin)[3] = '\0';

  return OK;
}

static STAT_Val parse_state_transitions(SPN_Span line, HT_HashTable * state_name_idx_map, State * state) {
  CHECK(!SPN_is_empty(line));
  CHECK(line.element_size == sizeof(char));
  CHECK(line.len >= 16);
  CHECK(state_name_idx_map != NULL);
  CHECK(state != NULL);

  const SPN_Span left_transition_name  = SPN_subspan(line, 7, 3);
  const SPN_Span right_transition_name = SPN_subspan(line, 12, 3);

  SPN_Span map_out_span = {0};
  TRY(HT_get(state_name_idx_map, left_transition_name, &map_out_span));
  CHECK(!SPN_is_empty(map_out_span));
  CHECK(map_out_span.element_size == sizeof(size_t));
  CHECK(map_out_span.len == 1);

  state->out_transitions[LEFT] = *((const size_t *)map_out_span.begin);

  map_out_span = (SPN_Span){0};
  TRY(HT_get(state_name_idx_map, right_transition_name, &map_out_span));
  CHECK(!SPN_is_empty(map_out_span));
  CHECK(map_out_span.element_size == sizeof(size_t));
  CHECK(map_out_span.len == 1);

  state->out_transitions[RIGHT] = *((const size_t *)map_out_span.begin);

  return OK;
}

STAT_Val parse_state_machine(SPN_Span lines, StateMachine * machine) {
  CHECK(!SPN_is_empty(lines));
  CHECK(lines.element_size == sizeof(DAR_DArray));
  CHECK(machine != NULL);

  TRY(init_machine(machine));

  TRY(DAR_reserve(&machine->states, lines.len));

  HT_HashTable state_name_idx_map = {0}; // will map state name to state index
  TRY(HT_create(&state_name_idx_map));

  // first retrieve all the state names, this is so we can easily convert from name to index later
  for(size_t i = 0; i < lines.len; i++) {
    const SPN_Span line      = DAR_to_span(SPN_get(lines, i));
    const size_t   state_idx = i;

    State       state           = {0};
    SPN_MutSpan state_name_span = {.begin        = state.name,
                                   .element_size = sizeof(state.name[0]),
                                   .len          = (sizeof(state.name) / sizeof(state.name[0]))};

    TRY(parse_state_name(line, state_name_span));
    TRY(DAR_push_back(&machine->states, &state));

    SPN_Span name_key = SPN_subspan(SPN_mut_to_const(state_name_span), 0, 3);

    TRY(HT_set(&state_name_idx_map,
               name_key,
               (SPN_Span){.begin = &state_idx, .element_size = sizeof(state_idx), .len = 1}));

    if(SPN_equals(SPN_from_cstr(state.name), SPN_from_cstr("AAA"))) {
      machine->initial_state = state_idx;
    } else if(SPN_equals(SPN_from_cstr(state.name), SPN_from_cstr("ZZZ"))) {
      machine->end_state = state_idx;
    }
  }

  for(size_t i = 0; i < lines.len; i++) {
    const SPN_Span line  = DAR_to_span(SPN_get(lines, i));
    State *        state = DAR_get(&machine->states, i);

    TRY(parse_state_transitions(line, &state_name_idx_map, state));
  }

  CHECK(machine->initial_state != machine->end_state);

  TRY(HT_destroy(&state_name_idx_map));

  return OK;
}

STAT_Val destroy_state_machine(StateMachine * machine) {
  CHECK(machine != NULL);

  TRY(DAR_destroy(&machine->states));
  *machine = (StateMachine){0};

  return OK;
}

STAT_Val parse_input_sequence(SPN_Span line, DAR_DArray * sequence) {
  CHECK(!SPN_is_empty(line));
  CHECK(sequence != NULL);
  CHECK(DAR_is_initialized(sequence));
  CHECK(sequence->element_size == sizeof(TransitionType));
  CHECK(DAR_is_empty(sequence));

  TRY(DAR_reserve(sequence, line.len));

  for(const char * p = SPN_first(line); p != SPN_end(line); p++) {
    if(*p != 'L' && *p != 'R') break; // assume sequence has ended
    TransitionType t = (*p == 'L') ? LEFT : RIGHT;
    TRY(DAR_push_back(sequence, &t));
  }

  return OK;
}

STAT_Val get_number_of_steps_for_input_on_state_machine(const StateMachine * machine,
                                                        SPN_Span             sequence,
                                                        size_t *             number_of_steps) {
  CHECK(machine != NULL);
  CHECK(DAR_is_initialized(&machine->states));
  CHECK(!DAR_is_empty(&machine->states));
  CHECK(machine->initial_state != machine->end_state);
  CHECK(!SPN_is_empty(sequence));
  CHECK(number_of_steps != NULL);

  *number_of_steps = 0;

  size_t state_idx = machine->initial_state;
  size_t input_idx = 0;

  while(state_idx != machine->end_state) {
    CHECK(state_idx < machine->states.size);
    const State * state = DAR_get(&machine->states, state_idx);

    state_idx = state->out_transitions[*(const TransitionType *)SPN_get(sequence, input_idx)];
    input_idx++;
    if(input_idx == sequence.len) input_idx = 0;

    (*number_of_steps)++;
  }

  return OK;
}