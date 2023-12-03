#include "lib.h"

#include <stdlib.h>

#include <cfac/test_utils.h>

#include <cfac/span.h>

static Result setup(void ** env_pp);
static Result teardown(void ** env_pp);

static Result tst_get_game_id(void) {
  Result r = PASS;

  GameResult result = {0};

  EXPECT_OK(&r, get_game_result_from_line(SPN_from_cstr("Game 1:"), &result));
  EXPECT_EQ(&r, 1, result.game_id);

  EXPECT_OK(&r, get_game_result_from_line(SPN_from_cstr("Game 9:"), &result));
  EXPECT_EQ(&r, 9, result.game_id);

  EXPECT_OK(&r, get_game_result_from_line(SPN_from_cstr("Game 53:"), &result));
  EXPECT_EQ(&r, 53, result.game_id);

  return r;
}

static Result tst_single_set_single_color(void) {
  Result r = PASS;

  GameResult result = {0};

  EXPECT_OK(&r, get_game_result_from_line(SPN_from_cstr("Game 1: 4 red"), &result));
  EXPECT_EQ(&r, 1, result.game_id);
  EXPECT_EQ(&r, 4, result.min_color_occurrences[RED]);

  EXPECT_OK(&r, get_game_result_from_line(SPN_from_cstr("Game 3: 9 green"), &result));
  EXPECT_EQ(&r, 3, result.game_id);
  EXPECT_EQ(&r, 9, result.min_color_occurrences[GREEN]);

  EXPECT_OK(&r, get_game_result_from_line(SPN_from_cstr("Game 10: 6 blue"), &result));
  EXPECT_EQ(&r, 10, result.game_id);
  EXPECT_EQ(&r, 6, result.min_color_occurrences[BLUE]);

  return r;
}

static Result tst_single_set_multi_colors(void) {
  Result r = PASS;

  GameResult result = {0};

  EXPECT_OK(&r, get_game_result_from_line(SPN_from_cstr("Game 12: 9 red, 2 green"), &result));
  EXPECT_EQ(&r, 12, result.game_id);
  EXPECT_EQ(&r, 9, result.min_color_occurrences[RED]);
  EXPECT_EQ(&r, 2, result.min_color_occurrences[GREEN]);

  EXPECT_OK(&r, get_game_result_from_line(SPN_from_cstr("Game 39: 8 red, 12 green, 5 blue"), &result));
  EXPECT_EQ(&r, 39, result.game_id);
  EXPECT_EQ(&r, 8, result.min_color_occurrences[RED]);
  EXPECT_EQ(&r, 12, result.min_color_occurrences[GREEN]);
  EXPECT_EQ(&r, 5, result.min_color_occurrences[BLUE]);

  return r;
}

static Result tst_multi_set_multi_colors(void) {
  Result r = PASS;

  GameResult result = {0};

  EXPECT_OK(&r, get_game_result_from_line(SPN_from_cstr("Game 12: 9 red, 2 green; 10 blue, 8 green"), &result));
  EXPECT_EQ(&r, 12, result.game_id);
  EXPECT_EQ(&r, 9, result.min_color_occurrences[RED]);
  EXPECT_EQ(&r, 8, result.min_color_occurrences[GREEN]);
  EXPECT_EQ(&r, 10, result.min_color_occurrences[BLUE]);

  EXPECT_OK(&r,
            get_game_result_from_line(SPN_from_cstr("Game 12: 1 red, 3 green; 4 blue, 2 green; 10 red, 8 green"),
                                      &result));
  EXPECT_EQ(&r, 12, result.game_id);
  EXPECT_EQ(&r, 10, result.min_color_occurrences[RED]);
  EXPECT_EQ(&r, 8, result.min_color_occurrences[GREEN]);
  EXPECT_EQ(&r, 4, result.min_color_occurrences[BLUE]);

  return r;
}

static Result tst_examples(void) {
  Result r = PASS;

  GameResult result = {0};

  // Game 1: 3 blue, 4 red; 1 red, 2 green, 6 blue; 2 green
  // Game 2: 1 blue, 2 green; 3 green, 4 blue, 1 red; 1 green, 1 blue
  // Game 3: 8 green, 6 blue, 20 red; 5 blue, 4 red, 13 green; 5 green, 1 red
  // Game 4: 1 green, 3 red, 6 blue; 3 green, 6 red; 3 green, 15 blue, 14 red
  // Game 5: 6 red, 1 blue, 3 green; 2 blue, 1 red, 2 green

  EXPECT_OK(&r,
            get_game_result_from_line(SPN_from_cstr("Game 1: 3 blue, 4 red; 1 red, 2 green, 6 blue; 2 green"),
                                      &result));
  EXPECT_EQ(&r, 1, result.game_id);
  EXPECT_EQ(&r, 6, result.min_color_occurrences[BLUE]);
  EXPECT_EQ(&r, 2, result.min_color_occurrences[GREEN]);
  EXPECT_EQ(&r, 4, result.min_color_occurrences[RED]);

  EXPECT_OK(&r,
            get_game_result_from_line(SPN_from_cstr("Game 2: 1 blue, 2 green; 3 green, 4 blue, 1 red; 1 green, 1 blue"),
                                      &result));
  EXPECT_EQ(&r, 2, result.game_id);
  EXPECT_EQ(&r, 1, result.min_color_occurrences[RED]);
  EXPECT_EQ(&r, 3, result.min_color_occurrences[GREEN]);
  EXPECT_EQ(&r, 4, result.min_color_occurrences[BLUE]);

  EXPECT_OK(&r,
            get_game_result_from_line(SPN_from_cstr(
                                          "Game 3: 8 green, 6 blue, 20 red; 5 blue, 4 red, 13 green; 5 green, 1 red"),
                                      &result));
  EXPECT_EQ(&r, 3, result.game_id);
  EXPECT_EQ(&r, 20, result.min_color_occurrences[RED]);
  EXPECT_EQ(&r, 13, result.min_color_occurrences[GREEN]);
  EXPECT_EQ(&r, 6, result.min_color_occurrences[BLUE]);

  EXPECT_OK(&r,
            get_game_result_from_line(SPN_from_cstr(
                                          "Game 4: 1 green, 3 red, 6 blue; 3 green, 6 red; 3 green, 15 blue, 14 red"),
                                      &result));
  EXPECT_EQ(&r, 4, result.game_id);
  EXPECT_EQ(&r, 14, result.min_color_occurrences[RED]);
  EXPECT_EQ(&r, 3, result.min_color_occurrences[GREEN]);
  EXPECT_EQ(&r, 15, result.min_color_occurrences[BLUE]);

  EXPECT_OK(&r,
            get_game_result_from_line(SPN_from_cstr("Game 5: 6 red, 1 blue, 3 green; 2 blue, 1 red, 2 green"),
                                      &result));
  EXPECT_EQ(&r, 5, result.game_id);
  EXPECT_EQ(&r, 6, result.min_color_occurrences[RED]);
  EXPECT_EQ(&r, 3, result.min_color_occurrences[GREEN]);
  EXPECT_EQ(&r, 2, result.min_color_occurrences[BLUE]);

  return r;
}

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_get_game_id,
      tst_single_set_single_color,
      tst_single_set_multi_colors,
      tst_multi_set_multi_colors,
      tst_examples,
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
