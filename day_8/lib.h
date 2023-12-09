#ifndef lib_h
#define lib_h

#include <cfac/darray.h>
#include <cfac/stat.h>

typedef enum TransitionType {
  LEFT,
  RIGHT,
  NUM_TRANSITION_TYPES,
} TransitionType;

typedef struct State {
  char   name[4]; // 3 characters plus terminator
  size_t out_transitions[NUM_TRANSITION_TYPES];
} State;

typedef struct StateMachine {
  DAR_DArray states; // contains State
  size_t     initial_state;
  size_t     end_state;
} StateMachine;

STAT_Val parse_state_machine(SPN_Span lines, StateMachine * machine);

STAT_Val destroy_state_machine(StateMachine * machine);

STAT_Val parse_input_sequence(SPN_Span line, DAR_DArray * sequence);

STAT_Val get_number_of_steps_for_input_on_state_machine_part1(const StateMachine * machine,
                                                              SPN_Span             sequence,
                                                              size_t *             number_of_steps);

STAT_Val get_number_of_steps_for_input_on_state_machine_part2(const StateMachine * machine,
                                                              SPN_Span             sequence,
                                                              size_t *             number_of_steps);

#endif