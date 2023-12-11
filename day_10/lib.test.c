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

static Result tst_parse_sketch_example_1(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "-L|F7\n",
      "7S-7|\n",
      "L|7||\n",
      "-L-J|\n",
      "L|-JF\n",
  };
  const Piece sketch_raw[] = {{char_to_piece_type('-'), SIZE_MAX, 0}, {char_to_piece_type('L'), SIZE_MAX, 0},
                              {char_to_piece_type('|'), SIZE_MAX, 0}, {char_to_piece_type('F'), SIZE_MAX, 0},
                              {char_to_piece_type('7'), SIZE_MAX, 0}, {char_to_piece_type('7'), SIZE_MAX, 0},
                              {char_to_piece_type('S'), SIZE_MAX, 0}, {char_to_piece_type('-'), SIZE_MAX, 0},
                              {char_to_piece_type('7'), SIZE_MAX, 0}, {char_to_piece_type('|'), SIZE_MAX, 0},
                              {char_to_piece_type('L'), SIZE_MAX, 0}, {char_to_piece_type('|'), SIZE_MAX, 0},
                              {char_to_piece_type('7'), SIZE_MAX, 0}, {char_to_piece_type('|'), SIZE_MAX, 0},
                              {char_to_piece_type('|'), SIZE_MAX, 0}, {char_to_piece_type('-'), SIZE_MAX, 0},
                              {char_to_piece_type('L'), SIZE_MAX, 0}, {char_to_piece_type('-'), SIZE_MAX, 0},
                              {char_to_piece_type('J'), SIZE_MAX, 0}, {char_to_piece_type('|'), SIZE_MAX, 0},
                              {char_to_piece_type('L'), SIZE_MAX, 0}, {char_to_piece_type('|'), SIZE_MAX, 0},
                              {char_to_piece_type('-'), SIZE_MAX, 0}, {char_to_piece_type('J'), SIZE_MAX, 0},
                              {char_to_piece_type('F'), SIZE_MAX, 0}};

  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, 5, &lines));

  PipeSketch sketch = {0};
  EXPECT_OK(&r, parse_sketch(&lines, &sketch));

  EXPECT_EQ(&r, sketch.width, 5);
  EXPECT_EQ(&r, sketch.height, 5);
  EXPECT_EQ(&r, sketch.pieces.size, 5 * 5);
  EXPECT_EQ(&r, 1, sketch.start_pos.x);
  EXPECT_EQ(&r, 1, sketch.start_pos.y);
  EXPECT_TRUE(&r, memcmp(sketch_raw, sketch.pieces.data, sketch.pieces.size) == 0);

  EXPECT_OK(&r, destroy_lines(&lines));
  EXPECT_OK(&r, destroy_sketch(&sketch));

  return r;
}

static Result tst_calculate_distances_from_start_example_1(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "-L|F7\n",
      "7S-7|\n",
      "L|7||\n",
      "-L-J|\n",
      "L|-JF\n",
  };
  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, 5, &lines));

  PipeSketch sketch = {0};
  EXPECT_OK(&r, parse_sketch(&lines, &sketch));

  EXPECT_OK(&r, calculate_distances_from_start(&sketch));
  EXPECT_EQ(&r, 0, get_piece(&sketch, sketch.start_pos)->dist_from_start);
  EXPECT_EQ(&r, 0, get_piece(&sketch, ((Position){1, 1}))->dist_from_start);
  EXPECT_EQ(&r, 1, get_piece(&sketch, ((Position){2, 1}))->dist_from_start);
  EXPECT_EQ(&r, 2, get_piece(&sketch, ((Position){3, 1}))->dist_from_start);
  EXPECT_EQ(&r, 1, get_piece(&sketch, ((Position){1, 2}))->dist_from_start);
  EXPECT_EQ(&r, 3, get_piece(&sketch, ((Position){3, 2}))->dist_from_start);
  EXPECT_EQ(&r, 2, get_piece(&sketch, ((Position){1, 3}))->dist_from_start);
  EXPECT_EQ(&r, 3, get_piece(&sketch, ((Position){2, 3}))->dist_from_start);
  EXPECT_EQ(&r, 4, get_piece(&sketch, ((Position){3, 3}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){0, 0}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){1, 0}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){2, 0}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){3, 0}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){4, 0}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){0, 1}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){4, 1}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){0, 2}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){2, 2}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){4, 2}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){0, 3}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){4, 3}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){0, 4}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){1, 4}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){2, 4}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){3, 4}))->dist_from_start);
  EXPECT_EQ(&r, SIZE_MAX, get_piece(&sketch, ((Position){4, 4}))->dist_from_start);

  EXPECT_OK(&r, destroy_lines(&lines));
  EXPECT_OK(&r, destroy_sketch(&sketch));

  return r;
}

static Result tst_get_max_distance_from_start_example_1(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "-L|F7\n",
      "7S-7|\n",
      "L|7||\n",
      "-L-J|\n",
      "L|-JF\n",
  };
  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, 5, &lines));

  PipeSketch sketch = {0};
  EXPECT_OK(&r, parse_sketch(&lines, &sketch));

  EXPECT_OK(&r, calculate_distances_from_start(&sketch));

  size_t   max_dist     = 0;
  Position max_dist_pos = {0};
  EXPECT_OK(&r, get_max_distance_from_start(&sketch, &max_dist, &max_dist_pos));

  EXPECT_EQ(&r, 4, max_dist);
  EXPECT_TRUE(&r, is_positions_equal((Position){3, 3}, max_dist_pos));

  EXPECT_OK(&r, destroy_lines(&lines));
  EXPECT_OK(&r, destroy_sketch(&sketch));

  return r;
}

static Result tst_get_max_distance_from_start_example_2(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "7-F7-\n",
      ".FJ|7\n",
      "SJLL7\n",
      "|F--J\n",
      "LJ.LJ\n",
  };
  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, 5, &lines));

  PipeSketch sketch = {0};
  EXPECT_OK(&r, parse_sketch(&lines, &sketch));

  EXPECT_OK(&r, calculate_distances_from_start(&sketch));

  size_t   max_dist     = 0;
  Position max_dist_pos = {0};
  EXPECT_OK(&r, get_max_distance_from_start(&sketch, &max_dist, &max_dist_pos));

  EXPECT_EQ(&r, 8, max_dist);
  EXPECT_TRUE(&r, is_positions_equal((Position){4, 2}, max_dist_pos));

  EXPECT_OK(&r, destroy_lines(&lines));
  EXPECT_OK(&r, destroy_sketch(&sketch));

  return r;
}

static Result tst_determine_enclosed_tiles_example_1(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "...........\n",
      ".S-------7.\n",
      ".|F-----7|.\n",
      ".||.....||.\n",
      ".||.....||.\n",
      ".|L-7.F-J|.\n",
      ".|..|.|..|.\n",
      ".L--J.L--J.\n",
      "...........\n",
  };
  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  PipeSketch sketch = {0};
  EXPECT_OK(&r, parse_sketch(&lines, &sketch));

  EXPECT_OK(&r, calculate_distances_from_start(&sketch));

  size_t num_enclosed_tiles = 0;
  EXPECT_OK(&r, determine_enclosed_tiles(&sketch, &num_enclosed_tiles));
  EXPECT_EQ(&r, 4, num_enclosed_tiles);

  EXPECT_OK(&r, destroy_lines(&lines));
  EXPECT_OK(&r, destroy_sketch(&sketch));

  return r;
}

static Result tst_determine_enclosed_tiles_example_2(void) {
  Result r = PASS;

  const char * lines_raw[] = {
      "..........\n",
      ".S------7.\n",
      ".|F----7|.\n",
      ".||....||.\n",
      ".||....||.\n",
      ".|L-7F-J|.\n",
      ".|..||..|.\n",
      ".L--JL--J.\n",
      "..........\n",
  };
  DAR_DArray lines = {0};
  EXPECT_OK(&r, create_lines(lines_raw, (sizeof(lines_raw) / sizeof(lines_raw[0])), &lines));

  PipeSketch sketch = {0};
  EXPECT_OK(&r, parse_sketch(&lines, &sketch));

  EXPECT_OK(&r, calculate_distances_from_start(&sketch));

  size_t num_enclosed_tiles = 0;
  EXPECT_OK(&r, determine_enclosed_tiles(&sketch, &num_enclosed_tiles));
  EXPECT_EQ(&r, 4, num_enclosed_tiles);

  EXPECT_OK(&r, destroy_lines(&lines));
  EXPECT_OK(&r, destroy_sketch(&sketch));

  return r;
}

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_parse_sketch_example_1,
      tst_calculate_distances_from_start_example_1,
      tst_get_max_distance_from_start_example_1,
      tst_get_max_distance_from_start_example_2,
      tst_determine_enclosed_tiles_example_1,
      tst_determine_enclosed_tiles_example_2,
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