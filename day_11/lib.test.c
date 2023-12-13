#include "lib.h"

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

static Result tst_parse_universe_example(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "...#......\n",
      ".......#..\n",
      "#.........\n",
      "..........\n",
      "......#...\n",
      ".#........\n",
      ".........#\n",
      "..........\n",
      ".......#..\n",
      "#...#.....\n",
  };
  const SpaceType emp          = EMPTY;
  const SpaceType gal          = GALAXY;
  const Space     spaces_raw[] = {
      // clang-format off
      {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {gal, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, 
      {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {gal, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, 
      {gal, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, 
      {emp, 1, 2}, {emp, 1, 2}, {emp, 2, 2}, {emp, 1, 2}, {emp, 1, 2}, {emp, 2, 2}, {emp, 1, 2}, {emp, 1, 2}, {emp, 2, 2}, {emp, 1, 2},
      {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {gal, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, 
      {emp, 1, 1}, {gal, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, 
      {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {gal, 1, 1}, 
      {emp, 1, 2}, {emp, 1, 2}, {emp, 2, 2}, {emp, 1, 2}, {emp, 1, 2}, {emp, 2, 2}, {emp, 1, 2}, {emp, 1, 2}, {emp, 2, 2}, {emp, 1, 2},
      {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {gal, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, 
      {gal, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {gal, 1, 1}, {emp, 2, 1}, {emp, 1, 1}, {emp, 1, 1}, {emp, 2, 1}, {emp, 1, 1},
      // clang-format on
  };
  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  Universe universe = {0};

  EXPECT_OK(&r, parse_universe(&lines, &universe));
  EXPECT_EQ(&r, 10, universe.width);
  EXPECT_EQ(&r, 10, universe.height);
  EXPECT_EQ(&r, 10 * 10, universe.spaces.size);

  EXPECT_TRUE(&r, memcmp(spaces_raw, universe.spaces.data, universe.spaces.size) == 0);

  if(HAS_FAILED(&r)) print_universe(&universe);

  EXPECT_OK(&r, destroy_lines(&lines));
  EXPECT_OK(&r, destroy_universe(&universe));

  return r;
}

static Result tst_calculate_galaxy_distances_example(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "...#......\n", // 3,0
      ".......#..\n", // 7,1
      "#.........\n", // 0,2
      "..........\n",
      "......#...\n", // 6,4
      ".#........\n", // 1,5
      ".........#\n", // 9,6
      "..........\n",
      ".......#..\n", // 7,8
      "#...#.....\n", // 0,9 ; 4,9
  };
  const Position expect_galaxy_positions[] = {{3, 0}, {7, 1}, {0, 2}, {6, 4}, {1, 5}, {9, 6}, {7, 8}, {0, 9}, {4, 9}};

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  Universe universe = {0};

  EXPECT_OK(&r, parse_universe(&lines, &universe));

  DAR_DArray galaxy_positions = {0};
  DAR_DArray distances        = {0};
  EXPECT_OK(&r, DAR_create(&galaxy_positions, sizeof(Position)));
  EXPECT_OK(&r, DAR_create(&distances, sizeof(size_t)));

  EXPECT_OK(&r, calculate_galaxy_distances(&universe, &galaxy_positions, &distances));
  EXPECT_EQ(&r, 9, galaxy_positions.size);
  EXPECT_EQ(&r, 9 * 9, distances.size);

  EXPECT_TRUE(&r, memcmp(expect_galaxy_positions, galaxy_positions.data, galaxy_positions.size) == 0);

  size_t sum_of_distances = 0;
  EXPECT_OK(&r, sum_galaxy_distances(&universe, &distances, galaxy_positions.size, &sum_of_distances));

  EXPECT_EQ(&r, sum_of_distances, 374);

  if(HAS_FAILED(&r)) print_universe(&universe);

  EXPECT_OK(&r, DAR_destroy(&galaxy_positions));
  EXPECT_OK(&r, DAR_destroy(&distances));

  EXPECT_OK(&r, destroy_lines(&lines));
  EXPECT_OK(&r, destroy_universe(&universe));

  return r;
}

static Result tst_calculate_galaxy_distances_example_part2(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "...#......\n", // 3,0
      ".......#..\n", // 7,1
      "#.........\n", // 0,2
      "..........\n",
      "......#...\n", // 6,4
      ".#........\n", // 1,5
      ".........#\n", // 9,6
      "..........\n",
      ".......#..\n", // 7,8
      "#...#.....\n", // 0,9 ; 4,9
  };
  const Position expect_galaxy_positions[] = {{3, 0}, {7, 1}, {0, 2}, {6, 4}, {1, 5}, {9, 6}, {7, 8}, {0, 9}, {4, 9}};

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  Universe universe = {0};

  EXPECT_OK(&r, parse_universe(&lines, &universe));

  DAR_DArray galaxy_positions = {0};
  DAR_DArray distances        = {0};
  EXPECT_OK(&r, DAR_create(&galaxy_positions, sizeof(Position)));
  EXPECT_OK(&r, DAR_create(&distances, sizeof(size_t)));

  EXPECT_OK(&r, increase_space_density_for_gaps(&universe, 10));

  EXPECT_OK(&r, calculate_galaxy_distances(&universe, &galaxy_positions, &distances));

  EXPECT_TRUE(&r, memcmp(expect_galaxy_positions, galaxy_positions.data, galaxy_positions.size) == 0);

  size_t sum_of_distances = 0;
  EXPECT_OK(&r, sum_galaxy_distances(&universe, &distances, galaxy_positions.size, &sum_of_distances));
  EXPECT_EQ(&r, sum_of_distances, 1030);

  EXPECT_OK(&r, increase_space_density_for_gaps(&universe, 100));
  EXPECT_OK(&r, DAR_clear(&galaxy_positions));
  EXPECT_OK(&r, DAR_clear(&distances));

  EXPECT_OK(&r, calculate_galaxy_distances(&universe, &galaxy_positions, &distances));

  EXPECT_OK(&r, sum_galaxy_distances(&universe, &distances, galaxy_positions.size, &sum_of_distances));
  EXPECT_EQ(&r, sum_of_distances, 8410);

  if(HAS_FAILED(&r)) print_universe(&universe);

  EXPECT_OK(&r, DAR_destroy(&galaxy_positions));
  EXPECT_OK(&r, DAR_destroy(&distances));

  EXPECT_OK(&r, destroy_lines(&lines));
  EXPECT_OK(&r, destroy_universe(&universe));

  return r;
}

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_parse_universe_example,
      tst_calculate_galaxy_distances_example,
      tst_calculate_galaxy_distances_example_part2,
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