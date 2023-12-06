#include "lib.h"

#include <stdlib.h>

#include <cfac/test_utils.h>

static Result setup(void ** env_pp);
static Result teardown(void ** env_pp);

static Result tst_get_card_score_basic(void) {
  Result r = PASS;

  SPN_Span card  = SPN_from_cstr("Card 1: 1 2 4 8 16 | 1 2 4 8 16");
  int      score = 0;

  EXPECT_OK(&r, get_card_score(card, &score));
  EXPECT_EQ(&r, 16, score);

  return r;
}

static Result tst_get_card_score_example(void) {
  Result r = PASS;

  SPN_Span cards[] = {
      SPN_from_cstr("Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53"),
      SPN_from_cstr("Card 2: 13 32 20 16 61 | 61 30 68 82 17 32 24 19"),
      SPN_from_cstr("Card 3:  1 21 53 59 44 | 69 82 63 72 16 21 14  1"),
      SPN_from_cstr("Card 4: 41 92 73 84 69 | 59 84 76 51 58  5 54 83"),
      SPN_from_cstr("Card 5: 87 83 26 28 32 | 88 30 70 12 93 22 82 36"),
      SPN_from_cstr("Card 6: 31 18 13 56 72 | 74 77 10 23 35 67 36 11"),
  };

  int total_score = 0;

  for(size_t i = 0; i < sizeof(cards) / sizeof(cards[0]); i++) {
    int score = 0;
    EXPECT_OK(&r, get_card_score(cards[i], &score));
    total_score += score;
  }

  EXPECT_EQ(&r, 13, total_score);

  return r;
}

static Result tst_get_total_number_of_cards_basic(void) {
  Result r = PASS;

  printf("start of %s\n", __func__);

  SPN_Span cards[] = {
      SPN_from_cstr("Card 1: 1 2 4 8 16 | 1 3 8 9 12"),  // win 1 and 8
      SPN_from_cstr("Card 2: 1 2 3 4 5 | 1 6 7 8 9"),    // win 1
      SPN_from_cstr("Card 3: 1 2 4 8 16 | 3 6 9 12 15"), // no win
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, DAR_create(&lines, sizeof(DAR_DArray)));

  for(size_t i = 0; i < sizeof(cards) / sizeof(cards[0]); i++) {
    DAR_DArray line = {0};
    EXPECT_OK(&r, DAR_create_from_span(&line, cards[i]));
    EXPECT_OK(&r, DAR_push_back(&lines, &line));
  }

  size_t num_of_cards = 0;

  EXPECT_OK(&r, get_total_number_of_cards(&lines, &num_of_cards));
  EXPECT_EQ(&r, 3 + 2 + 1 + 1, num_of_cards);

  for(DAR_DArray * line = DAR_first(&lines); line != DAR_end(&lines); line++) { EXPECT_OK(&r, DAR_destroy(line)); }
  EXPECT_OK(&r, DAR_destroy(&lines));

  return r;
}

static Result tst_get_total_number_of_cards_example(void) {
  Result r = PASS;

  printf("start of %s\n", __func__);

  SPN_Span cards[] = {
      SPN_from_cstr("Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53"),
      SPN_from_cstr("Card 2: 13 32 20 16 61 | 61 30 68 82 17 32 24 19"),
      SPN_from_cstr("Card 3:  1 21 53 59 44 | 69 82 63 72 16 21 14  1"),
      SPN_from_cstr("Card 4: 41 92 73 84 69 | 59 84 76 51 58  5 54 83"),
      SPN_from_cstr("Card 5: 87 83 26 28 32 | 88 30 70 12 93 22 82 36"),
      SPN_from_cstr("Card 6: 31 18 13 56 72 | 74 77 10 23 35 67 36 11"),
  };

  DAR_DArray lines = {0};
  EXPECT_OK(&r, DAR_create(&lines, sizeof(DAR_DArray)));

  for(size_t i = 0; i < sizeof(cards) / sizeof(cards[0]); i++) {
    DAR_DArray line = {0};
    EXPECT_OK(&r, DAR_create_from_span(&line, cards[i]));
    EXPECT_OK(&r, DAR_push_back(&lines, &line));
  }

  size_t num_of_cards = 0;

  EXPECT_OK(&r, get_total_number_of_cards(&lines, &num_of_cards));
  EXPECT_EQ(&r, 30, num_of_cards);

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
      tst_get_card_score_basic,
      tst_get_card_score_example,
      tst_get_total_number_of_cards_basic,
      tst_get_total_number_of_cards_example,
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