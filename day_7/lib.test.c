#include "lib.h"

#include <stdlib.h>

#include <cfac/test_utils.h>

static Result setup(void ** env_pp);
static Result teardown(void ** env_pp);

static Hand hand_from_cstr(const char * cstr) { return parse_hand(SPN_from_cstr(cstr)); }

static Result tst_get_hand_type(void) {
  Result r = PASS;

  EXPECT_EQ(&r, FIVE_OF_A_KIND, get_hand_type(hand_from_cstr("55555 0")));
  EXPECT_EQ(&r, FIVE_OF_A_KIND, get_hand_type(hand_from_cstr("AAAAA 0")));
  EXPECT_EQ(&r, FIVE_OF_A_KIND, get_hand_type(hand_from_cstr("KKKKK 0")));

  EXPECT_EQ(&r, FOUR_OF_A_KIND, get_hand_type(hand_from_cstr("KKKKA 0")));
  EXPECT_EQ(&r, FOUR_OF_A_KIND, get_hand_type(hand_from_cstr("KKAKK 0")));
  EXPECT_EQ(&r, FOUR_OF_A_KIND, get_hand_type(hand_from_cstr("28222 0")));
  EXPECT_EQ(&r, FOUR_OF_A_KIND, get_hand_type(hand_from_cstr("44A44 0")));

  EXPECT_EQ(&r, FULL_HOUSE, get_hand_type(hand_from_cstr("KKKAA 0")));
  EXPECT_EQ(&r, FULL_HOUSE, get_hand_type(hand_from_cstr("KKAKA 0")));
  EXPECT_EQ(&r, FULL_HOUSE, get_hand_type(hand_from_cstr("9A9AA 0")));
  EXPECT_EQ(&r, FULL_HOUSE, get_hand_type(hand_from_cstr("9AA9A 0")));
  EXPECT_EQ(&r, FULL_HOUSE, get_hand_type(hand_from_cstr("3J3JJ 0")));
  EXPECT_EQ(&r, FULL_HOUSE, get_hand_type(hand_from_cstr("QQ55Q 0")));

  EXPECT_EQ(&r, THREE_OF_A_KIND, get_hand_type(hand_from_cstr("QQ54Q 0")));
  EXPECT_EQ(&r, THREE_OF_A_KIND, get_hand_type(hand_from_cstr("QQQ54 0")));
  EXPECT_EQ(&r, THREE_OF_A_KIND, get_hand_type(hand_from_cstr("QQ54Q 0")));
  EXPECT_EQ(&r, THREE_OF_A_KIND, get_hand_type(hand_from_cstr("QQ5Q4 0")));
  EXPECT_EQ(&r, THREE_OF_A_KIND, get_hand_type(hand_from_cstr("Q5QQ4 0")));

  EXPECT_EQ(&r, TWO_PAIR, get_hand_type(hand_from_cstr("Q5Q45 0")));
  EXPECT_EQ(&r, TWO_PAIR, get_hand_type(hand_from_cstr("A66A4 0")));
  EXPECT_EQ(&r, TWO_PAIR, get_hand_type(hand_from_cstr("66JJ4 0")));
  EXPECT_EQ(&r, TWO_PAIR, get_hand_type(hand_from_cstr("66949 0")));

  EXPECT_EQ(&r, ONE_PAIR, get_hand_type(hand_from_cstr("66943 0")));
  EXPECT_EQ(&r, ONE_PAIR, get_hand_type(hand_from_cstr("65993 0")));
  EXPECT_EQ(&r, ONE_PAIR, get_hand_type(hand_from_cstr("559A3 0")));
  EXPECT_EQ(&r, ONE_PAIR, get_hand_type(hand_from_cstr("5J9AA 0")));
  EXPECT_EQ(&r, ONE_PAIR, get_hand_type(hand_from_cstr("5J339 0")));

  EXPECT_EQ(&r, HIGH_CARD, get_hand_type(hand_from_cstr("5J349 0")));
  EXPECT_EQ(&r, HIGH_CARD, get_hand_type(hand_from_cstr("24938 0")));
  EXPECT_EQ(&r, HIGH_CARD, get_hand_type(hand_from_cstr("2A9T8 0")));
  EXPECT_EQ(&r, HIGH_CARD, get_hand_type(hand_from_cstr("7AKT9 0")));

  return r;
}

static Result tst_compare_hands(void) {
  Result r = PASS;

  EXPECT_EQ(&r, 0, compare_hands(hand_from_cstr("AAAAA 0"), hand_from_cstr("AAAAA 9")));
  EXPECT_EQ(&r, 0, compare_hands(hand_from_cstr("JJ359 0"), hand_from_cstr("JJ359 9")));
  EXPECT_EQ(&r, 0, compare_hands(hand_from_cstr("23456 0"), hand_from_cstr("23456 9")));
  EXPECT_EQ(&r, 0, compare_hands(hand_from_cstr("29TT8 0"), hand_from_cstr("29TT8 9")));
  EXPECT_EQ(&r, 0, compare_hands(hand_from_cstr("2TTT8 0"), hand_from_cstr("2TTT8 9")));
  EXPECT_EQ(&r, 0, compare_hands(hand_from_cstr("8TTT8 0"), hand_from_cstr("8TTT8 9")));

  // unequal based on type
  EXPECT_EQ(&r, -1, compare_hands(hand_from_cstr("AAAAJ 0"), hand_from_cstr("AAAAA 9")));
  EXPECT_EQ(&r, -1, compare_hands(hand_from_cstr("AAAJJ 0"), hand_from_cstr("AAAAA 9")));
  EXPECT_EQ(&r, -1, compare_hands(hand_from_cstr("AAAJJ 0"), hand_from_cstr("AAAAJ 9")));
  EXPECT_EQ(&r, -1, compare_hands(hand_from_cstr("448AJ 0"), hand_from_cstr("448JJ 9")));
  EXPECT_EQ(&r, 1, compare_hands(hand_from_cstr("448AJ 0"), hand_from_cstr("438J6 9")));
  EXPECT_EQ(&r, 1, compare_hands(hand_from_cstr("448A8 0"), hand_from_cstr("438JJ 9")));
  EXPECT_EQ(&r, 1, compare_hands(hand_from_cstr("444A8 0"), hand_from_cstr("438JJ 9")));
  EXPECT_EQ(&r, 1, compare_hands(hand_from_cstr("444AA 0"), hand_from_cstr("444JA 9")));

  // unequal based on individual cards
  EXPECT_EQ(&r, -1, compare_hands(hand_from_cstr("447JJ 0"), hand_from_cstr("448JJ 9")));
  EXPECT_EQ(&r, -1, compare_hands(hand_from_cstr("JJJJJ 0"), hand_from_cstr("AAAAA 9")));
  EXPECT_EQ(&r, -1, compare_hands(hand_from_cstr("JJJ44 0"), hand_from_cstr("AAA88 9")));
  EXPECT_EQ(&r, 1, compare_hands(hand_from_cstr("JJA44 0"), hand_from_cstr("JJQ22 9")));
  EXPECT_EQ(&r, 1, compare_hands(hand_from_cstr("23567 0"), hand_from_cstr("23564 9")));

  return r;
}

static Result tst_sort_hands_by_rank_example(void) {
  Result r = PASS;

  const Hand hands_in_order[] = {
      hand_from_cstr("32T3K 765\n"),
      hand_from_cstr("KTJJT 220\n"),
      hand_from_cstr("KK677 28\n"),
      hand_from_cstr("T55J5 684\n"),
      hand_from_cstr("QQQJA 483\n"),
  };

  Hand hands[] = {
      hands_in_order[4],
      hands_in_order[2],
      hands_in_order[0],
      hands_in_order[1],
      hands_in_order[3],
  };

  SPN_MutSpan hands_span = {.begin = hands, .element_size = sizeof(Hand), .len = (sizeof(hands) / sizeof(hands[0]))};

  EXPECT_OK(&r, sort_hands_by_rank(hands_span));

  EXPECT_TRUE(&r, hand_equals(hands[0], hands_in_order[0]));
  EXPECT_TRUE(&r, hand_equals(hands[1], hands_in_order[1]));
  EXPECT_TRUE(&r, hand_equals(hands[2], hands_in_order[2]));
  EXPECT_TRUE(&r, hand_equals(hands[3], hands_in_order[3]));
  EXPECT_TRUE(&r, hand_equals(hands[4], hands_in_order[4]));

  return r;
}

static Result tst_get_total_winnings_example(void) {
  Result r = PASS;

  const Hand hands_in_order[] = {
      hand_from_cstr("32T3K 765\n"),
      hand_from_cstr("KTJJT 220\n"),
      hand_from_cstr("KK677 28\n"),
      hand_from_cstr("T55J5 684\n"),
      hand_from_cstr("QQQJA 483\n"),
  };

  Hand hands[] = {
      hands_in_order[4],
      hands_in_order[2],
      hands_in_order[0],
      hands_in_order[1],
      hands_in_order[3],
  };

  SPN_MutSpan hands_span = {.begin = hands, .element_size = sizeof(Hand), .len = (sizeof(hands) / sizeof(hands[0]))};

  size_t total_winnings = 0;
  EXPECT_OK(&r, get_total_winnings(hands_span, &total_winnings));

  EXPECT_EQ(&r, 6440, total_winnings);

  return r;
}

static Result tst_fixture(void * env) {
  Result r = PASS;

  EXPECT_NE(&r, NULL, env);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_get_hand_type,
      tst_compare_hands,
      tst_sort_hands_by_rank_example,
      tst_get_total_winnings_example,
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