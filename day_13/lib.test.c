#include "lib.h"

#include "common.h"

#include <stdlib.h>

#include <cfac/test_utils.h>

static Result setup(void ** env_pp);
static Result teardown(void ** env_pp);

static STAT_Val create_lines(const char ** lines_raw, size_t n, DAR_DArray * lines_darr) {
  TRY(DAR_create(lines_darr, sizeof(DAR_DArray)));

  for(size_t i = 0; i < n; i++) {
    DAR_DArray line = {0};
    TRY(DAR_create_from_cstr(&line, lines_raw[i]));
    TRY(DAR_push_back(lines_darr, &line));
  }

  return OK;
}

static STAT_Val destroy_lines(DAR_DArray * lines) {
  CHECK(lines != NULL);

  for(DAR_DArray * p = DAR_first(lines); p != DAR_end(lines); p++) { TRY(DAR_destroy(p)); }

  TRY(DAR_destroy(lines));

  return OK;
}

static Result tst_parse_pattern_example_1(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "#.##..##.\n", // 101100110 -> 0x166
      "..#.##.#.\n", // 001011010 -> 0x5a
      "##......#\n", // 110000001 -> 0x181
      "##......#\n", // 110000001 -> 0x181
      "..#.##.#.\n", // 001011010 -> 0x5a
      "..##..##.\n", // 001100110 -> 0x66
      "#.#.##.#.\n", // 101011010 -> 0x15a
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  Pattern pattern = {0};
  EXPECT_OK(&r, parse_pattern(DAR_to_span(&lines), &pattern));
  EXPECT_EQ(&r, 7, pattern.rows.size);

  EXPECT_ARREQ(&r,
               size_t,
               ((size_t[]){0x166, 0x5a, 0x181, 0x181, 0x5a, 0x66, 0x15a}),
               pattern.rows.data,
               pattern.rows.size);

  EXPECT_OK(&r, destroy_pattern(&pattern));
  EXPECT_OK(&r, destroy_lines(&lines));

  return r;
}

static Result tst_transpose_pattern_example_1(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "#.##..##.\n", // 101100110 -> 0x166
      "..#.##.#.\n", // 001011010 -> 0x5a
      "##......#\n", // 110000001 -> 0x181
      "##......#\n", // 110000001 -> 0x181
      "..#.##.#.\n", // 001011010 -> 0x5a
      "..##..##.\n", // 001100110 -> 0x66
      "#.#.##.#.\n", // 101011010 -> 0x15a
                     // transposed:
                     // 1011001 -> 0x59
                     // 0011001 -> 0x19
                     // 1100111 -> 0x67
                     // 1000010 -> 0x42
                     // 0100101 -> 0x25
                     // 0100101 -> 0x25
                     // 1000010 -> 0x42
                     // 1100111 -> 0x67
                     // 0011000 -> 0x18
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  Pattern pattern = {0};
  EXPECT_OK(&r, parse_pattern(DAR_to_span(&lines), &pattern));

  Pattern pattern_transp = {0};
  EXPECT_OK(&r, transpose_pattern(&pattern, &pattern_transp));
  EXPECT_EQ(&r, pattern_transp.width, pattern.rows.size);

  EXPECT_ARREQ(&r,
               size_t,
               ((size_t[]){0x59, 0x19, 0x67, 0x42, 0x25, 0x25, 0x42, 0x67, 0x18}),
               pattern_transp.rows.data,
               pattern_transp.rows.size);

  EXPECT_OK(&r, destroy_pattern(&pattern));
  EXPECT_OK(&r, destroy_pattern(&pattern_transp));
  EXPECT_OK(&r, destroy_lines(&lines));

  return r;
}

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_parse_pattern_example_1,
      tst_transpose_pattern_example_1,
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