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

  EXPECT_EQ(&r, 1, almanac.maps[SEED_TO_SOIL].size);
  EXPECT_EQ(&r, 1, almanac.maps[SOIL_TO_FERTILIZER].size);
  EXPECT_EQ(&r, 1, almanac.maps[FERTILIZER_TO_WATER].size);
  EXPECT_EQ(&r, 1, almanac.maps[WATER_TO_LIGHT].size);
  EXPECT_EQ(&r, 1, almanac.maps[LIGHT_TO_TEMPERATURE].size);
  EXPECT_EQ(&r, 1, almanac.maps[TEMPERATURE_TO_HUMIDITY].size);
  EXPECT_EQ(&r, 1, almanac.maps[HUMIDITY_TO_LOCATION].size);

  EXPECT_EQ(&r, 50, ((MapRange *)DAR_first(&almanac.maps[SEED_TO_SOIL]))->dst_start);
  EXPECT_EQ(&r, 98, ((MapRange *)DAR_first(&almanac.maps[SEED_TO_SOIL]))->src_start);
  EXPECT_EQ(&r, 2, ((MapRange *)DAR_first(&almanac.maps[SEED_TO_SOIL]))->length);

  EXPECT_EQ(&r, 0, ((MapRange *)DAR_first(&almanac.maps[SOIL_TO_FERTILIZER]))->dst_start);
  EXPECT_EQ(&r, 15, ((MapRange *)DAR_first(&almanac.maps[SOIL_TO_FERTILIZER]))->src_start);
  EXPECT_EQ(&r, 37, ((MapRange *)DAR_first(&almanac.maps[SOIL_TO_FERTILIZER]))->length);

  EXPECT_EQ(&r, 49, ((MapRange *)DAR_first(&almanac.maps[FERTILIZER_TO_WATER]))->dst_start);
  EXPECT_EQ(&r, 53, ((MapRange *)DAR_first(&almanac.maps[FERTILIZER_TO_WATER]))->src_start);
  EXPECT_EQ(&r, 8, ((MapRange *)DAR_first(&almanac.maps[FERTILIZER_TO_WATER]))->length);

  EXPECT_EQ(&r, 88, ((MapRange *)DAR_first(&almanac.maps[WATER_TO_LIGHT]))->dst_start);
  EXPECT_EQ(&r, 18, ((MapRange *)DAR_first(&almanac.maps[WATER_TO_LIGHT]))->src_start);
  EXPECT_EQ(&r, 7, ((MapRange *)DAR_first(&almanac.maps[WATER_TO_LIGHT]))->length);

  EXPECT_EQ(&r, 45, ((MapRange *)DAR_first(&almanac.maps[LIGHT_TO_TEMPERATURE]))->dst_start);
  EXPECT_EQ(&r, 77, ((MapRange *)DAR_first(&almanac.maps[LIGHT_TO_TEMPERATURE]))->src_start);
  EXPECT_EQ(&r, 23, ((MapRange *)DAR_first(&almanac.maps[LIGHT_TO_TEMPERATURE]))->length);

  EXPECT_EQ(&r, 0, ((MapRange *)DAR_first(&almanac.maps[TEMPERATURE_TO_HUMIDITY]))->dst_start);
  EXPECT_EQ(&r, 69, ((MapRange *)DAR_first(&almanac.maps[TEMPERATURE_TO_HUMIDITY]))->src_start);
  EXPECT_EQ(&r, 1, ((MapRange *)DAR_first(&almanac.maps[TEMPERATURE_TO_HUMIDITY]))->length);

  EXPECT_EQ(&r, 60, ((MapRange *)DAR_first(&almanac.maps[HUMIDITY_TO_LOCATION]))->dst_start);
  EXPECT_EQ(&r, 56, ((MapRange *)DAR_first(&almanac.maps[HUMIDITY_TO_LOCATION]))->src_start);
  EXPECT_EQ(&r, 37, ((MapRange *)DAR_first(&almanac.maps[HUMIDITY_TO_LOCATION]))->length);

  EXPECT_OK(&r, destroy_almanac(&almanac));

  for(DAR_DArray * line = DAR_first(&lines); line != DAR_end(&lines); line++) { EXPECT_OK(&r, DAR_destroy(line)); }
  EXPECT_OK(&r, DAR_destroy(&lines));

  return r;
}

static Result tst_find_lowest_location_example(void) {
  Result r = PASS;

  const char * raw_lines[] = {
      "seeds: 79 14 55 13\n",
      "\n",
      "seed-to-soil map:\n",
      "50 98 2\n",
      "52 50 48\n",
      "\n",
      "soil-to-fertilizer map:\n",
      "0 15 37\n",
      "37 52 2\n",
      "39 0 15\n",
      "\n",
      "fertilizer-to-water map:\n",
      "49 53 8\n",
      "0 11 42\n",
      "42 0 7\n",
      "57 7 4\n",
      "\n",
      "water-to-light map:\n",
      "88 18 7\n",
      "18 25 70\n",
      "\n",
      "light-to-temperature map:\n",
      "45 77 23\n",
      "81 45 19\n",
      "68 64 13\n",
      "\n",
      "temperature-to-humidity map:\n",
      "0 69 1\n",
      "1 0 69\n",
      "\n",
      "humidity-to-location map:\n",
      "60 56 37\n",
      "56 93 4\n",
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

  size_t lowest_location = 0;
  EXPECT_OK(&r, find_lowest_location_number(&almanac, &lowest_location));
  EXPECT_EQ(&r, 35, lowest_location);

  if(HAS_FAILED(&r)) printf("lowest_location: %zu\n", lowest_location);

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
      tst_find_lowest_location_example,
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