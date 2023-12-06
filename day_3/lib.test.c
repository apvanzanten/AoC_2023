#include "lib.h"

#include <stdlib.h>

#include <cfac/test_utils.h>

static Result setup(void ** env_pp);
static Result teardown(void ** env_pp);

static Result tst_get_numbers_from_schematic(void) {
  Result r = PASS;

  {
    DAR_DArray schematic_arr = {0};
    EXPECT_OK(&r, DAR_create(&schematic_arr, sizeof(SPN_Span)));

    {
      SPN_Span lines[] = {
          SPN_from_cstr("123...454."),
          SPN_from_cstr("...x12...."),
          SPN_from_cstr("....8....."),
      };

      for(size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
        SPN_Span * line = &lines[i];
        EXPECT_OK(&r, DAR_push_back(&schematic_arr, line));
      }
    }

    DAR_DArray numbers = {0};
    EXPECT_OK(&r, DAR_create(&numbers, sizeof(int)));

    EXPECT_OK(&r, get_numbers_from_schematic(DAR_to_span(&schematic_arr), &numbers));
    EXPECT_EQ(&r, 3, numbers.size);
    EXPECT_EQ(&r, 123, *(int*)DAR_get(&numbers, 0));
    EXPECT_EQ(&r, 12, *(int*)DAR_get(&numbers, 1));
    EXPECT_EQ(&r, 8, *(int*)DAR_get(&numbers, 2));

    EXPECT_OK(&r, DAR_destroy(&numbers));
    EXPECT_OK(&r, DAR_destroy(&schematic_arr));
  }

  {
    DAR_DArray schematic_arr = {0};
    EXPECT_OK(&r, DAR_create(&schematic_arr, sizeof(SPN_Span)));

    {
      SPN_Span lines[] = {
          SPN_from_cstr("467..114.."),
          SPN_from_cstr("...*......"),
          SPN_from_cstr("..35..633."),
          SPN_from_cstr("......#..."),
          SPN_from_cstr("617*......"),
          SPN_from_cstr(".....+.58."),
          SPN_from_cstr("..592....."),
          SPN_from_cstr("......755."),
          SPN_from_cstr("...$.*...."),
          SPN_from_cstr(".664.598.."),
      };

      for(size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
        SPN_Span * line = &lines[i];
        EXPECT_OK(&r, DAR_push_back(&schematic_arr, line));
      }
    }

    DAR_DArray numbers = {0};
    EXPECT_OK(&r, DAR_create(&numbers, sizeof(int)));

    EXPECT_OK(&r, get_numbers_from_schematic(DAR_to_span(&schematic_arr), &numbers));
    EXPECT_EQ(&r, 8, numbers.size);

    int num_sum = 0;
    for(int * p = DAR_first(&numbers); p != DAR_end(&numbers); p++) {
      num_sum += *p;
      printf("found: %d\n", *p);
    }
    EXPECT_EQ(&r, num_sum, 4361);

    EXPECT_OK(&r, DAR_destroy(&numbers));
    EXPECT_OK(&r, DAR_destroy(&schematic_arr));
  }

  return r;
}

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_get_numbers_from_schematic,
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