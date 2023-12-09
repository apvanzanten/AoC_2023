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

  CHECK((machine->initial_state != machine->end_state) || (machine->initial_state == 0));

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

STAT_Val get_number_of_steps_for_input_on_state_machine_part1(const StateMachine * machine,
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

static bool is_state_ghost_start_state(const State * state) { return state->name[2] == 'A'; }

static bool is_state_ghost_end_state(const State * state) { return state->name[2] == 'Z'; }

static STAT_Val get_ghost_start_states(const StateMachine * machine, DAR_DArray * state_indices) {
  CHECK(machine != NULL);
  CHECK(state_indices != NULL);
  CHECK(state_indices->element_size == sizeof(size_t));
  CHECK(DAR_is_empty(state_indices));

  for(size_t i = 0; i < machine->states.size; i++) {
    if(is_state_ghost_start_state(DAR_get(&machine->states, i))) { TRY(DAR_push_back(state_indices, &i)); }
  }

  return OK;
}

typedef struct GhostPosition {
  size_t state_idx;
  size_t input_idx;
} GhostPosition;

typedef struct Cycle {
  size_t start;
  size_t length;
} Cycle;

static STAT_Val get_cycle_in_history(const DAR_DArray * history, Cycle * cycle) {
  CHECK(history != NULL);
  if(history->size <= 1) return STAT_OK_NOT_FOUND;

  const GhostPosition current_pos = *(const GhostPosition *)DAR_last(history);

  for(size_t i = 0; i < (history->size - 1); i++) {
    const GhostPosition pos = *(const GhostPosition *)DAR_get(history, i);

    if((pos.state_idx == current_pos.state_idx) && (pos.input_idx == current_pos.input_idx)) {
      cycle->start  = i;
      cycle->length = ((history->size - 1) - i);
      return STAT_OK;
    }
  }

  return STAT_OK_NOT_FOUND;
}

static STAT_Val find_cycle(const StateMachine * machine,
                           SPN_Span             input_seq,
                           size_t               start_idx,
                           Cycle *              cycle,
                           size_t *             first_end_state) {
  CHECK(machine != NULL);
  CHECK(cycle != NULL);

  *cycle = (Cycle){0};

  DAR_DArray history = {0};
  TRY(DAR_create(&history, sizeof(GhostPosition)));

  GhostPosition pos = {.state_idx = start_idx, .input_idx = 0};

  STAT_Val stat_get_cycle = STAT_OK;

  size_t step        = 0;
  (*first_end_state) = SIZE_MAX;

  while((stat_get_cycle = get_cycle_in_history(&history, cycle)) == STAT_OK_NOT_FOUND) {
    const State * state = DAR_get(&machine->states, pos.state_idx);

    if((*first_end_state) == SIZE_MAX && is_state_ghost_end_state(state)) { (*first_end_state) = step; }

    TRY(DAR_push_back(&history, &pos));

    const TransitionType * input = SPN_get(input_seq, pos.input_idx);

    pos.state_idx = state->out_transitions[*input];

    pos.input_idx++;
    if(pos.input_idx == input_seq.len) pos.input_idx = 0;

    step++;
  }

  CHECK(STAT_is_OK(stat_get_cycle));

  TRY(DAR_destroy(&history));

  return OK;
}

static size_t gcd(size_t a, size_t b) {
  // textbook euclid's
  size_t t = 0;
  while(b != 0) {
    t = b;
    b = (a % b);
    a = t;
  }
  return a;
}

static STAT_Val get_least_common_multiple_for_cycle_lengths(const DAR_DArray * cycles_darr,
                                                            size_t *           least_common_multiple) {
  CHECK(cycles_darr != NULL);
  CHECK(cycles_darr->element_size == sizeof(Cycle));

  // assertions:
  // * lcm(a,b) = a * (b / gcd(a,b))
  // * lcm(a,b,c) = lcm(a, lcm(b,c))
  // * lcm(1,b) = b
  // * euclid is pretty cool guy, he determines gcd and doesn't afraid of anything

  const Cycle * cycles = cycles_darr->data;

  size_t lcm = 1;
  for(size_t i = 0; i < cycles_darr->size; i++) {
    const size_t a = lcm;
    const size_t b = cycles[i].length;
    lcm            = a * (b / gcd(a, b));
  }

  *least_common_multiple = lcm;

  return OK;
}

STAT_Val get_number_of_steps_for_input_on_state_machine_part2(const StateMachine * machine,
                                                              SPN_Span             sequence,
                                                              size_t *             number_of_steps) {
  CHECK(machine != NULL);
  CHECK(DAR_is_initialized(&machine->states));
  CHECK(!DAR_is_empty(&machine->states));
  CHECK((machine->initial_state != machine->end_state) || (machine->initial_state == 0));
  CHECK(!SPN_is_empty(sequence));
  CHECK(number_of_steps != NULL);

  *number_of_steps = 0;

  DAR_DArray ghost_state_indices = {0};
  TRY(DAR_create(&ghost_state_indices, sizeof(size_t)));
  TRY(get_ghost_start_states(machine, &ghost_state_indices));

  DAR_DArray cycles = {0};
  TRY(DAR_create(&cycles, sizeof(Cycle)));
  TRY(DAR_resize_zeroed(&cycles, ghost_state_indices.size));

  for(size_t i = 0; i < ghost_state_indices.size; i++) {
    const size_t * state_idx       = DAR_get(&ghost_state_indices, i);
    Cycle *        cycle           = DAR_get(&cycles, i);
    size_t         first_end_state = 0;
    TRY(find_cycle(machine, sequence, *state_idx, cycle, &first_end_state));
    LOG_STAT(STAT_OK,
             "found cycle for ghost %zu: {%zu, %zu}, with first end state: %zu",
             i,
             cycle->start,
             cycle->length,
             first_end_state);
  }

  // NOTE it turns out that in this input, all ghost cycles have only one end state, the position (in the overall
  // sequence) of which is equal to the size of the cycle, and thus kind of by definition must coincide with the end of
  // the input sequence. The upshot:
  // - each ghost reaches a valid end state at step (ghost_cycle_length * i), where i is any non-zero positive integer.
  // - we will find the the convergence at the least common multiple of the set of cycle lengths.

  TRY(get_least_common_multiple_for_cycle_lengths(&cycles, number_of_steps));

  TRY(DAR_destroy(&cycles));
  TRY(DAR_destroy(&ghost_state_indices));

  return OK;
}