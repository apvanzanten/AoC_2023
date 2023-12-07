#include "lib.h"

#include <stdlib.h>

#include <cfac/test_utils.h>

static Result setup(void ** env_pp);
static Result teardown(void ** env_pp);

static Result tst_parse_almanac_basic(void) {
  Result r = PASS;

  const char * raw_lines[] = {
      "seeds: 79 14\n", "\n", "seed-to-soil map:\n",         "50 98 2\n",  "\n", "soil-to-fertilizer map:\n",
      "0 15 37\n",      "\n", "fertilizer-to-water map:\n",  "49 53 8\n",  "\n", "water-to-light map:\n",
      "88 18 7\n",      "\n", "light-to-temperature map:\n", "45 77 23\n", "\n", "temperature-to-humidity map:\n",
      "0 69 1\n",       "\n", "humidity-to-location map:\n", "60 56 37\n",
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, DAR_create(&lines, sizeof(DAR_DArray)));
  for(size_t i = 0; i < sizeof(raw_lines) / sizeof(raw_lines[0]); i++) {
    DAR_DArray line = {0};
    EXPECT_OK(&r, DAR_create_from_cstr(&line, raw_lines[i]));
    EXPECT_OK(&r, DAR_push_back(&lines, &line));
  }

  Almanac almanac = {0};
  EXPECT_OK(&r, parse_almanac(&lines, &almanac));

  EXPECT_EQ(&r, 2, almanac.seeds.size);
  EXPECT_EQ(&r, 79, *(size_t *)DAR_get(&almanac.seeds, 0));
  EXPECT_EQ(&r, 14, *(size_t *)DAR_get(&almanac.seeds, 1));

  EXPECT_OK(&r, destroy_almanac(&almanac));

  for(DAR_DArray * line = DAR_first(&lines); line != DAR_end(&lines); line++) { EXPECT_OK(&r, DAR_destroy(line)); }
  EXPECT_OK(&r, DAR_destroy(&lines));

  return r;
}

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_parse_almanac_basic,
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