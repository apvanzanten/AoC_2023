#include "lib.h"

#include <stdlib.h>

#include <cfac/test_utils.h>

#include "common.h"

static Result setup(void ** env_pp);
static Result teardown(void ** env_pp);

static STAT_Val create_lines_arr(const char ** lines, size_t n, DAR_DArray * arr) {
  CHECK(lines != NULL);
  CHECK(arr != NULL);

  TRY(DAR_create(arr, sizeof(DAR_DArray)));

  for(size_t i = 0; i < n; i++) {
    const char * line     = lines[i];
    DAR_DArray   line_arr = {0};
    TRY(DAR_create_from_cstr(&line_arr, line));
    TRY(DAR_push_back(arr, &line_arr));
  }

  return OK;
}

static STAT_Val destroy_lines_arr(DAR_DArray * arr) {
  for(DAR_DArray * line = DAR_first(arr); line != DAR_end(arr); line++) { TRY(DAR_destroy(line)); }
  TRY(DAR_destroy(arr));

  return OK;
}

static Result tst_parse_state_machine_basic(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "AAA = (BBB, BBB)\n",
      "BBB = (AAA, ZZZ)\n",
      "ZZZ = (ZZZ, ZZZ)\n",
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines_arr(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  StateMachine machine = {0};

  EXPECT_OK(&r, parse_state_machine(DAR_to_span(&lines), &machine));

  EXPECT_EQ(&r, 3, machine.states.size);
  EXPECT_STREQ(&r, "AAA", ((State *)DAR_get(&machine.states, 0))->name);
  EXPECT_STREQ(&r, "BBB", ((State *)DAR_get(&machine.states, 1))->name);
  EXPECT_STREQ(&r, "ZZZ", ((State *)DAR_get(&machine.states, 2))->name);

  EXPECT_EQ(&r, 1, ((State *)DAR_get(&machine.states, 0))->out_transitions[LEFT]);
  EXPECT_EQ(&r, 1, ((State *)DAR_get(&machine.states, 0))->out_transitions[RIGHT]);

  EXPECT_EQ(&r, 0, ((State *)DAR_get(&machine.states, 1))->out_transitions[LEFT]);
  EXPECT_EQ(&r, 2, ((State *)DAR_get(&machine.states, 1))->out_transitions[RIGHT]);

  EXPECT_EQ(&r, 2, ((State *)DAR_get(&machine.states, 2))->out_transitions[LEFT]);
  EXPECT_EQ(&r, 2, ((State *)DAR_get(&machine.states, 2))->out_transitions[RIGHT]);

  EXPECT_OK(&r, destroy_state_machine(&machine));
  EXPECT_OK(&r, destroy_lines_arr(&lines));

  return r;
}

static Result tst_parse_state_machine_example(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "AAA = (BBB, CCC)\n",
      "BBB = (DDD, EEE)\n",
      "CCC = (ZZZ, GGG)\n",
      "DDD = (DDD, DDD)\n",
      "EEE = (EEE, EEE)\n",
      "GGG = (GGG, GGG)\n",
      "ZZZ = (ZZZ, ZZZ)\n",
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines_arr(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  StateMachine machine = {0};

  EXPECT_OK(&r, parse_state_machine(DAR_to_span(&lines), &machine));

  EXPECT_EQ(&r, 7, machine.states.size);
  EXPECT_STREQ(&r, "AAA", ((State *)DAR_get(&machine.states, 0))->name);
  EXPECT_STREQ(&r, "BBB", ((State *)DAR_get(&machine.states, 1))->name);
  EXPECT_STREQ(&r, "CCC", ((State *)DAR_get(&machine.states, 2))->name);
  EXPECT_STREQ(&r, "DDD", ((State *)DAR_get(&machine.states, 3))->name);
  EXPECT_STREQ(&r, "EEE", ((State *)DAR_get(&machine.states, 4))->name);
  EXPECT_STREQ(&r, "GGG", ((State *)DAR_get(&machine.states, 5))->name);
  EXPECT_STREQ(&r, "ZZZ", ((State *)DAR_get(&machine.states, 6))->name);

  // AAA
  EXPECT_EQ(&r, 1, ((State *)DAR_get(&machine.states, 0))->out_transitions[LEFT]);
  EXPECT_EQ(&r, 2, ((State *)DAR_get(&machine.states, 0))->out_transitions[RIGHT]);

  // BBB
  EXPECT_EQ(&r, 3, ((State *)DAR_get(&machine.states, 1))->out_transitions[LEFT]);
  EXPECT_EQ(&r, 4, ((State *)DAR_get(&machine.states, 1))->out_transitions[RIGHT]);

  // CCC
  EXPECT_EQ(&r, 6, ((State *)DAR_get(&machine.states, 2))->out_transitions[LEFT]);
  EXPECT_EQ(&r, 5, ((State *)DAR_get(&machine.states, 2))->out_transitions[RIGHT]);

  // DDD
  EXPECT_EQ(&r, 3, ((State *)DAR_get(&machine.states, 3))->out_transitions[LEFT]);
  EXPECT_EQ(&r, 3, ((State *)DAR_get(&machine.states, 3))->out_transitions[RIGHT]);

  // EEE
  EXPECT_EQ(&r, 4, ((State *)DAR_get(&machine.states, 4))->out_transitions[LEFT]);
  EXPECT_EQ(&r, 4, ((State *)DAR_get(&machine.states, 4))->out_transitions[RIGHT]);

  // GGG
  EXPECT_EQ(&r, 5, ((State *)DAR_get(&machine.states, 5))->out_transitions[LEFT]);
  EXPECT_EQ(&r, 5, ((State *)DAR_get(&machine.states, 5))->out_transitions[RIGHT]);

  // ZZZ
  EXPECT_EQ(&r, 6, ((State *)DAR_get(&machine.states, 6))->out_transitions[LEFT]);
  EXPECT_EQ(&r, 6, ((State *)DAR_get(&machine.states, 6))->out_transitions[RIGHT]);

  EXPECT_OK(&r, destroy_state_machine(&machine));
  EXPECT_OK(&r, destroy_lines_arr(&lines));

  return r;
}

static Result tst_parse_input_sequence(void) {
  Result r = PASS;

  DAR_DArray sequence = {0};
  EXPECT_OK(&r, DAR_create(&sequence, sizeof(TransitionType)));

  EXPECT_OK(&r, parse_input_sequence(SPN_from_cstr("L"), &sequence));
  EXPECT_EQ(&r, 1, sequence.size);
  EXPECT_EQ(&r, LEFT, *(TransitionType *)DAR_get(&sequence, 0));

  EXPECT_OK(&r, DAR_clear(&sequence));

  EXPECT_OK(&r, parse_input_sequence(SPN_from_cstr("LLRR"), &sequence));
  EXPECT_EQ(&r, 4, sequence.size);
  EXPECT_EQ(&r, LEFT, *(TransitionType *)DAR_get(&sequence, 0));
  EXPECT_EQ(&r, LEFT, *(TransitionType *)DAR_get(&sequence, 1));
  EXPECT_EQ(&r, RIGHT, *(TransitionType *)DAR_get(&sequence, 2));
  EXPECT_EQ(&r, RIGHT, *(TransitionType *)DAR_get(&sequence, 3));

  EXPECT_OK(&r, DAR_clear(&sequence));

  EXPECT_OK(&r, parse_input_sequence(SPN_from_cstr("LRLLLR"), &sequence));
  EXPECT_EQ(&r, 6, sequence.size);
  EXPECT_EQ(&r, LEFT, *(TransitionType *)DAR_get(&sequence, 0));
  EXPECT_EQ(&r, RIGHT, *(TransitionType *)DAR_get(&sequence, 1));
  EXPECT_EQ(&r, LEFT, *(TransitionType *)DAR_get(&sequence, 2));
  EXPECT_EQ(&r, LEFT, *(TransitionType *)DAR_get(&sequence, 3));
  EXPECT_EQ(&r, LEFT, *(TransitionType *)DAR_get(&sequence, 4));
  EXPECT_EQ(&r, RIGHT, *(TransitionType *)DAR_get(&sequence, 5));

  EXPECT_OK(&r, DAR_destroy(&sequence));

  return r;
}

static Result tst_get_number_of_steps_for_input_on_state_machine_part1_example_1(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "RL\n",
      "\n",
      "AAA = (BBB, CCC)\n",
      "BBB = (DDD, EEE)\n",
      "CCC = (ZZZ, GGG)\n",
      "DDD = (DDD, DDD)\n",
      "EEE = (EEE, EEE)\n",
      "GGG = (GGG, GGG)\n",
      "ZZZ = (ZZZ, ZZZ)\n",
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines_arr(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  DAR_DArray input_seq = {0};
  EXPECT_OK(&r, DAR_create(&input_seq, sizeof(TransitionType)));

  EXPECT_OK(&r, parse_input_sequence(DAR_to_span(DAR_first(&lines)), &input_seq));
  EXPECT_EQ(&r, 2, input_seq.size);

  StateMachine machine = {0};
  EXPECT_OK(&r, parse_state_machine(SPN_subspan(DAR_to_span(&lines), 2, lines.size - 2), &machine));

  size_t number_of_steps = 0;

  EXPECT_OK(&r,
            get_number_of_steps_for_input_on_state_machine_part1(&machine, DAR_to_span(&input_seq), &number_of_steps));
  EXPECT_EQ(&r, 2, number_of_steps);

  EXPECT_OK(&r, DAR_destroy(&input_seq));
  EXPECT_OK(&r, destroy_state_machine(&machine));
  EXPECT_OK(&r, destroy_lines_arr(&lines));

  return r;
}

static Result tst_get_number_of_steps_for_input_on_state_machine_part1_example_2(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "LLR\n",
      "\n",
      "AAA = (BBB, BBB)\n",
      "BBB = (AAA, ZZZ)\n",
      "ZZZ = (ZZZ, ZZZ)\n",
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines_arr(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  DAR_DArray input_seq = {0};
  EXPECT_OK(&r, DAR_create(&input_seq, sizeof(TransitionType)));

  EXPECT_OK(&r, parse_input_sequence(DAR_to_span(DAR_first(&lines)), &input_seq));
  EXPECT_EQ(&r, 3, input_seq.size);

  StateMachine machine = {0};
  EXPECT_OK(&r, parse_state_machine(SPN_subspan(DAR_to_span(&lines), 2, lines.size - 2), &machine));

  size_t number_of_steps = 0;

  EXPECT_OK(&r,
            get_number_of_steps_for_input_on_state_machine_part1(&machine, DAR_to_span(&input_seq), &number_of_steps));
  EXPECT_EQ(&r, 6, number_of_steps);

  EXPECT_OK(&r, DAR_destroy(&input_seq));
  EXPECT_OK(&r, destroy_state_machine(&machine));
  EXPECT_OK(&r, destroy_lines_arr(&lines));

  return r;
}

static Result tst_get_number_of_steps_for_input_on_state_machine_part2_example(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "LR\n",
      "\n",
      "11A = (11B, XXX)\n",
      "11B = (XXX, 11Z)\n",
      "11Z = (11B, XXX)\n",
      "22A = (22B, XXX)\n",
      "22B = (22C, 22C)\n",
      "22C = (22Z, 22Z)\n",
      "22Z = (22B, 22B)\n",
      "XXX = (XXX, XXX)\n",
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines_arr(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  DAR_DArray input_seq = {0};
  EXPECT_OK(&r, DAR_create(&input_seq, sizeof(TransitionType)));

  EXPECT_OK(&r, parse_input_sequence(DAR_to_span(DAR_first(&lines)), &input_seq));
  EXPECT_EQ(&r, 2, input_seq.size);

  StateMachine machine = {0};
  EXPECT_OK(&r, parse_state_machine(SPN_subspan(DAR_to_span(&lines), 2, lines.size - 2), &machine));

  size_t number_of_steps = 0;

  EXPECT_OK(&r,
            get_number_of_steps_for_input_on_state_machine_part2(&machine, DAR_to_span(&input_seq), &number_of_steps));
  EXPECT_EQ(&r, 6, number_of_steps);

  EXPECT_OK(&r, DAR_destroy(&input_seq));
  EXPECT_OK(&r, destroy_state_machine(&machine));
  EXPECT_OK(&r, destroy_lines_arr(&lines));

  return r;
}

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_parse_state_machine_basic,
      tst_parse_state_machine_example,
      tst_parse_input_sequence,
      tst_get_number_of_steps_for_input_on_state_machine_part1_example_1,
      tst_get_number_of_steps_for_input_on_state_machine_part1_example_2,
      tst_get_number_of_steps_for_input_on_state_machine_part2_example,
  };

  TestWithFixture tests_with_fixture[] = {
      tst_fixture,
  };

  const Result test_res = run_tests(tests, sizeof(tests) / sizeof(Test));
  const Result test_with_fixture_res =
      run_tests_with_fixture(tests_with_fixture, sizeof(tests_with_fixture) / sizeof(TestWithFixture), setup, teardown);

  return ((test_res == PASS) && (test_with_fixture_res == PASS)) ? 0 : 1;
}

static Result setup(void ** env_pp) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env_pp);
  if(HAS_FAILED(&r)) return r;

  int * i = malloc(sizeof(int));
  EXPECT_NE(&r, NULL, i);

  if(HAS_FAILED(&r)) free(i);

  *env_pp = i;

  return r;
}

static Result teardown(void ** env_pp) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env_pp);
  if(HAS_FAILED(&r)) return r;

  free(*env_pp);

  return r;
}