#include "lib.h"

#include <cfac/test_utils.h>

#include <stdlib.h>

#include "common.h"

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

static Result tst_parse_records_example(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "???.### 1,1,3\n",
      ".??..??...?##. 1,1,3\n",
      "?#?#?#?#?#?#?#? 1,3,1,6\n",
      "????.#...#... 4,1,1\n",
      "????.######..#####. 1,6,5\n",
      "?###???????? 3,2,1\n",
  };
  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  DAR_DArray records = {0};
  EXPECT_OK(&r, DAR_create(&records, sizeof(Record)));

  EXPECT_OK(&r, parse_records(&lines, &records));

  EXPECT_EQ(&r, lines.size, records.size);
  EXPECT_EQ(&r, 3, ((Record *)DAR_get(&records, 0))->groups.size);
  EXPECT_ARREQ(&r, size_t, ((size_t[]){1, 1, 3}), DAR_first(&((Record *)DAR_get(&records, 0))->groups), 3);

  EXPECT_EQ(&r, 3, ((Record *)DAR_get(&records, 1))->groups.size);
  EXPECT_ARREQ(&r, size_t, ((size_t[]){1, 1, 3}), DAR_first(&((Record *)DAR_get(&records, 1))->groups), 3);

  EXPECT_EQ(&r, 4, ((Record *)DAR_get(&records, 2))->groups.size);
  EXPECT_ARREQ(&r, size_t, ((size_t[]){1, 3, 1, 6}), DAR_first(&((Record *)DAR_get(&records, 2))->groups), 4);

  EXPECT_OK(&r, print_records(&records));

  EXPECT_OK(&r, destroy_records(&records));
  EXPECT_OK(&r, DAR_destroy(&records));
  EXPECT_OK(&r, destroy_lines(&lines));

  return r;
}

static Result tst_get_num_possibilities_for_record_example_line1(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "???.### 1,1,3\n",
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  DAR_DArray records = {0};
  EXPECT_OK(&r, DAR_create(&records, sizeof(Record)));

  EXPECT_OK(&r, parse_records(&lines, &records));

  EXPECT_EQ(&r, lines.size, records.size);

  size_t num_possibilities = 0;
  EXPECT_OK(&r, get_num_possibilities_for_record(DAR_first(&records), &num_possibilities));
  EXPECT_EQ(&r, 1, num_possibilities);

  EXPECT_OK(&r, destroy_records(&records));
  EXPECT_OK(&r, DAR_destroy(&records));
  EXPECT_OK(&r, destroy_lines(&lines));

  return r;
}

static Result tst_get_num_possibilities_for_record_example_line2(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      ".??..??...?##. 1,1,3\n",
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  DAR_DArray records = {0};
  EXPECT_OK(&r, DAR_create(&records, sizeof(Record)));

  EXPECT_OK(&r, parse_records(&lines, &records));

  EXPECT_EQ(&r, lines.size, records.size);

  size_t num_possibilities = 0;
  EXPECT_OK(&r, get_num_possibilities_for_record(DAR_first(&records), &num_possibilities));
  EXPECT_EQ(&r, 4, num_possibilities);

  EXPECT_OK(&r, destroy_records(&records));
  EXPECT_OK(&r, DAR_destroy(&records));
  EXPECT_OK(&r, destroy_lines(&lines));

  return r;
}

static Result tst_get_num_possibilities_for_record_example(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "???.### 1,1,3\n",
      ".??..??...?##. 1,1,3\n",
      "?#?#?#?#?#?#?#? 1,3,1,6\n",
      "????.#...#... 4,1,1\n",
      "????.######..#####. 1,6,5\n",
      "?###???????? 3,2,1\n",
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  DAR_DArray records = {0};
  EXPECT_OK(&r, DAR_create(&records, sizeof(Record)));

  EXPECT_OK(&r, parse_records(&lines, &records));

  EXPECT_EQ(&r, lines.size, records.size);

  size_t num_possibilities = 0;
  EXPECT_OK(&r, get_num_possibilities_for_record(DAR_get(&records, 0), &num_possibilities));
  EXPECT_EQ(&r, 1, num_possibilities);

  EXPECT_OK(&r, get_num_possibilities_for_record(DAR_get(&records, 1), &num_possibilities));
  EXPECT_EQ(&r, 4, num_possibilities);

  EXPECT_OK(&r, get_num_possibilities_for_record(DAR_get(&records, 2), &num_possibilities));
  EXPECT_EQ(&r, 1, num_possibilities);

  EXPECT_OK(&r, get_num_possibilities_for_record(DAR_get(&records, 3), &num_possibilities));
  EXPECT_EQ(&r, 1, num_possibilities);

  EXPECT_OK(&r, get_num_possibilities_for_record(DAR_get(&records, 4), &num_possibilities));
  EXPECT_EQ(&r, 4, num_possibilities);

  EXPECT_OK(&r, get_num_possibilities_for_record(DAR_get(&records, 5), &num_possibilities));
  EXPECT_EQ(&r, 10, num_possibilities);

  EXPECT_OK(&r, get_num_possibilities_for_all_records(&records, &num_possibilities));
  EXPECT_EQ(&r, 21, num_possibilities);

  EXPECT_OK(&r, destroy_records(&records));
  EXPECT_OK(&r, DAR_destroy(&records));
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
      tst_parse_records_example,
      tst_get_num_possibilities_for_record_example_line1,
      tst_get_num_possibilities_for_record_example_line2,
      tst_get_num_possibilities_for_record_example,
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