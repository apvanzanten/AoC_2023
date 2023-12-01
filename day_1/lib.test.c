#include "lib.h"

#include <stdlib.h>

#include <cfac/test_utils.h>

#include <cfac/span.h>


static Result setup(void ** env_pp);
static Result teardown(void ** env_pp);

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);

  return r;
}

static Result tst_get_calibration_value_part_a(void) {
  Result r = PASS;

  int value = 0;

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("12"), &value));
  EXPECT_EQ(&r, 12, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("254"), &value));
  EXPECT_EQ(&r, 24, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("25sdasere4"), &value));
  EXPECT_EQ(&r, 24, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("hi12bye"), &value));
  EXPECT_EQ(&r, 12, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("1abc2"), &value));
  EXPECT_EQ(&r, 12, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("pqr3stu8vwx"), &value));
  EXPECT_EQ(&r, 38, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("a1b2c3d4e5f"), &value));
  EXPECT_EQ(&r, 15, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("treb7uchet"), &value));
  EXPECT_EQ(&r, 77, value);

  return r;
}

static Result tst_get_calibration_value_part_b(void) {
  Result r = PASS;

  int value = 0;

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("12nine"), &value));
  EXPECT_EQ(&r, 19, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("four254"), &value));
  EXPECT_EQ(&r, 44, value);


  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("two1nine"), &value));
  EXPECT_EQ(&r, 29, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("eightwothree"), &value));
  EXPECT_EQ(&r, 83, value);
  if(HAS_FAILED(&r)) printf("%d\n", value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("abcone2threexyz"), &value));
  EXPECT_EQ(&r, 13, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("xtwone3four"), &value));
  EXPECT_EQ(&r, 24, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("4nineeightseven2"), &value));
  EXPECT_EQ(&r, 42, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("zoneight234"), &value));
  EXPECT_EQ(&r, 14, value);

  EXPECT_OK(&r, get_calibration_value(SPN_from_cstr("7pqrstsixteen"), &value));
  EXPECT_EQ(&r, 76, value);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_get_calibration_value_part_a,
      tst_get_calibration_value_part_b,
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