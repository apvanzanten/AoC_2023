#include "lib.h"

#include <stdlib.h>

#include <cfac/test_utils.h>

#include <cfac/darray.h>

static Result setup(void ** env_pp);
static Result teardown(void ** env_pp);

static Result tst_get_delta_sequence_example(void) {
  Result r = PASS;

  const ssize_t start_seq[] = {10, 13, 16, 21, 30, 45};
  const ssize_t seq_step1[] = {3, 3, 5, 9, 15};
  const ssize_t seq_step2[] = {0, 2, 4, 6};
  const ssize_t seq_step3[] = {2, 2, 2};
  const ssize_t seq_step4[] = {0, 0};

  DAR_DArray seq_a = {0};
  DAR_DArray seq_b = {0};
  EXPECT_OK(&r, DAR_create(&seq_a, sizeof(ssize_t)));
  EXPECT_OK(&r, DAR_create(&seq_b, sizeof(ssize_t)));

  EXPECT_OK(&r, DAR_push_back_array(&seq_a, start_seq, (sizeof(start_seq) / sizeof(start_seq[0]))));
  EXPECT_OK(&r, DAR_resize_zeroed(&seq_b, seq_a.size - 1));

  EXPECT_OK(&r, get_delta_sequence(DAR_to_span(&seq_a), DAR_to_mut_span(&seq_b)));
  EXPECT_ARREQ(&r, ssize_t, seq_step1, seq_b.data, seq_b.size);

  EXPECT_OK(&r, DAR_clear(&seq_a));
  EXPECT_OK(&r, DAR_push_back_darray(&seq_a, &seq_b));
  EXPECT_OK(&r, DAR_clear(&seq_b));
  EXPECT_OK(&r, DAR_resize_zeroed(&seq_b, seq_a.size - 1));

  EXPECT_OK(&r, get_delta_sequence(DAR_to_span(&seq_a), DAR_to_mut_span(&seq_b)));
  EXPECT_ARREQ(&r, ssize_t, seq_step2, seq_b.data, seq_b.size);

  EXPECT_OK(&r, DAR_clear(&seq_a));
  EXPECT_OK(&r, DAR_push_back_darray(&seq_a, &seq_b));
  EXPECT_OK(&r, DAR_clear(&seq_b));
  EXPECT_OK(&r, DAR_resize_zeroed(&seq_b, seq_a.size - 1));

  EXPECT_OK(&r, get_delta_sequence(DAR_to_span(&seq_a), DAR_to_mut_span(&seq_b)));
  EXPECT_ARREQ(&r, ssize_t, seq_step3, seq_b.data, seq_b.size);

  EXPECT_OK(&r, DAR_clear(&seq_a));
  EXPECT_OK(&r, DAR_push_back_darray(&seq_a, &seq_b));
  EXPECT_OK(&r, DAR_clear(&seq_b));
  EXPECT_OK(&r, DAR_resize_zeroed(&seq_b, seq_a.size - 1));

  EXPECT_OK(&r, get_delta_sequence(DAR_to_span(&seq_a), DAR_to_mut_span(&seq_b)));
  EXPECT_ARREQ(&r, ssize_t, seq_step4, seq_b.data, seq_b.size);

  EXPECT_OK(&r, DAR_destroy(&seq_a));
  EXPECT_OK(&r, DAR_destroy(&seq_b));

  return r;
}

static Result tst_generate_histories_for_sequence_example(void) {
  Result r = PASS;

  const ssize_t start_seq[] = {10, 13, 16, 21, 30, 45};
  const ssize_t seq_step1[] = {3, 3, 5, 9, 15};
  const ssize_t seq_step2[] = {0, 2, 4, 6};
  const ssize_t seq_step3[] = {2, 2, 2};
  const ssize_t seq_step4[] = {0, 0};

  SPN_Span start_span = {.begin        = start_seq,
                         .element_size = sizeof(ssize_t),
                         .len          = (sizeof(start_seq) / sizeof(start_seq[0]))};

  DAR_DArray histories = {0};
  EXPECT_OK(&r, DAR_create(&histories, sizeof(DAR_DArray)));

  EXPECT_OK(&r, generate_histories_for_sequence(start_span, &histories));

  EXPECT_EQ(&r, 4, histories.size);
  EXPECT_EQ(&r, (sizeof(seq_step1) / sizeof(ssize_t)), ((DAR_DArray *)DAR_get(&histories, 0))->size);
  EXPECT_EQ(&r, (sizeof(seq_step2) / sizeof(ssize_t)), ((DAR_DArray *)DAR_get(&histories, 1))->size);
  EXPECT_EQ(&r, (sizeof(seq_step3) / sizeof(ssize_t)), ((DAR_DArray *)DAR_get(&histories, 2))->size);
  EXPECT_EQ(&r, (sizeof(seq_step4) / sizeof(ssize_t)), ((DAR_DArray *)DAR_get(&histories, 3))->size);

  EXPECT_ARREQ(&r,
               ssize_t,
               seq_step1,
               ((DAR_DArray *)DAR_get(&histories, 0))->data,
               ((DAR_DArray *)DAR_get(&histories, 0))->size);
  EXPECT_ARREQ(&r,
               ssize_t,
               seq_step2,
               ((DAR_DArray *)DAR_get(&histories, 1))->data,
               ((DAR_DArray *)DAR_get(&histories, 1))->size);
  EXPECT_ARREQ(&r,
               ssize_t,
               seq_step3,
               ((DAR_DArray *)DAR_get(&histories, 2))->data,
               ((DAR_DArray *)DAR_get(&histories, 2))->size);
  EXPECT_ARREQ(&r,
               ssize_t,
               seq_step4,
               ((DAR_DArray *)DAR_get(&histories, 3))->data,
               ((DAR_DArray *)DAR_get(&histories, 3))->size);

  EXPECT_OK(&r, destroy_histories(&histories));

  return r;
}

static Result tst_get_next_value_in_sequence_example(void) {
  Result r = PASS;

  const ssize_t start_seq[] = {10, 13, 16, 21, 30, 45};

  SPN_Span sequence = {.begin        = start_seq,
                       .element_size = sizeof(ssize_t),
                       .len          = (sizeof(start_seq) / sizeof(start_seq[0]))};

  ssize_t next_value = 0;
  EXPECT_OK(&r, get_next_value_in_sequence(sequence, &next_value));
  EXPECT_EQ(&r, 68, next_value);

  return r;
}

static Result tst_get_prev_value_in_sequence_example(void) {
  Result r = PASS;

  const ssize_t start_seq[] = {10, 13, 16, 21, 30, 45};

  SPN_Span sequence = {.begin        = start_seq,
                       .element_size = sizeof(ssize_t),
                       .len          = (sizeof(start_seq) / sizeof(start_seq[0]))};

  ssize_t prev_value = 0;
  EXPECT_OK(&r, get_prev_value_in_sequence(sequence, &prev_value));
  EXPECT_EQ(&r, 5, prev_value);

  return r;
}

static Result tst_get_sum_of_next_values_in_sequences_example(void) {
  Result r = PASS;

  const ssize_t seq_1[] = {0, 3, 6, 9, 12, 15};
  const ssize_t seq_2[] = {1, 3, 6, 10, 15, 21};
  const ssize_t seq_3[] = {10, 13, 16, 21, 30, 45};

  SPN_Span sequences_arr[] = {
      {.begin = seq_1, .element_size = sizeof(ssize_t), .len = (sizeof(seq_1) / sizeof(seq_1[0]))},
      {.begin = seq_2, .element_size = sizeof(ssize_t), .len = (sizeof(seq_2) / sizeof(seq_2[0]))},
      {.begin = seq_3, .element_size = sizeof(ssize_t), .len = (sizeof(seq_3) / sizeof(seq_3[0]))},
  };

  SPN_Span sequences = {.begin = sequences_arr, .element_size = sizeof(SPN_Span), .len = 3};

  ssize_t sum_of_next_values = 0;
  EXPECT_OK(&r, get_sum_of_next_values_in_sequences(sequences, &sum_of_next_values));
  EXPECT_EQ(&r, 114, sum_of_next_values);

  return r;
}

static Result tst_get_sum_of_prev_values_in_sequences_example(void) {
  Result r = PASS;

  const ssize_t seq_1[] = {0, 3, 6, 9, 12, 15};
  const ssize_t seq_2[] = {1, 3, 6, 10, 15, 21};
  const ssize_t seq_3[] = {10, 13, 16, 21, 30, 45};

  SPN_Span sequences_arr[] = {
      {.begin = seq_1, .element_size = sizeof(ssize_t), .len = (sizeof(seq_1) / sizeof(seq_1[0]))},
      {.begin = seq_2, .element_size = sizeof(ssize_t), .len = (sizeof(seq_2) / sizeof(seq_2[0]))},
      {.begin = seq_3, .element_size = sizeof(ssize_t), .len = (sizeof(seq_3) / sizeof(seq_3[0]))},
  };

  SPN_Span sequences = {.begin = sequences_arr, .element_size = sizeof(SPN_Span), .len = 3};

  ssize_t sum_of_prev_values = 0;
  EXPECT_OK(&r, get_sum_of_prev_values_in_sequences(sequences, &sum_of_prev_values));
  EXPECT_EQ(&r, 2, sum_of_prev_values);

  return r;
}

static Result tst_parse_sequence_line_example(void) {
  Result r = PASS;

  const ssize_t seq_1[] = {0, 3, 6, 9, 12, 15};
  const ssize_t seq_2[] = {1, 3, 6, 10, 15, 21};
  const ssize_t seq_3[] = {10, 13, 16, 21, 30, 45};

  const char * lines[] = {
      "0 3 6 9 12 15\n",
      "1 3 6 10 15 21\n",
      "10 13 16 21 30 45\n",
  };

  DAR_DArray sequence = {0};
  EXPECT_OK(&r, DAR_create(&sequence, sizeof(ssize_t)));

  EXPECT_OK(&r, parse_sequence_line(SPN_from_cstr(lines[0]), &sequence));
  EXPECT_EQ(&r, sizeof(seq_1) / sizeof(seq_1[0]), sequence.size);
  EXPECT_ARREQ(&r, ssize_t, seq_1, sequence.data, sequence.size);

  EXPECT_OK(&r, DAR_clear(&sequence));

  EXPECT_OK(&r, parse_sequence_line(SPN_from_cstr(lines[1]), &sequence));
  EXPECT_EQ(&r, sizeof(seq_2) / sizeof(seq_2[0]), sequence.size);
  EXPECT_ARREQ(&r, ssize_t, seq_2, sequence.data, sequence.size);

  EXPECT_OK(&r, DAR_clear(&sequence));

  EXPECT_OK(&r, parse_sequence_line(SPN_from_cstr(lines[2]), &sequence));
  EXPECT_EQ(&r, sizeof(seq_3) / sizeof(seq_3[0]), sequence.size);
  EXPECT_ARREQ(&r, ssize_t, seq_3, sequence.data, sequence.size);

  EXPECT_OK(&r, DAR_destroy(&sequence));

  return r;
}

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_get_delta_sequence_example,
      tst_generate_histories_for_sequence_example,
      tst_get_next_value_in_sequence_example,
      tst_get_prev_value_in_sequence_example,
      tst_get_sum_of_next_values_in_sequences_example,
      tst_get_sum_of_prev_values_in_sequences_example,
      tst_parse_sequence_line_example,
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