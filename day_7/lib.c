#include <cfac/log.h>
#include <cfac/span.h>

#include "common.h"
#include "lib.h"

#include <stdio.h>
#include <stdlib.h>

static Card char_to_card(char c) {
  if(c >= '2' && c <= '9') return (Card)(c - '2' + TWO);
  switch(c) {
  case 'A': return ACE;
  case 'K': return KING;
  case 'Q': return QUEEN;
  case 'J': return JACK;
  case 'T': return TEN;
  default: LOG_STAT(STAT_ERR_ARGS, "invalid card char: %c", c);
  }
  return TWO;
}

Hand parse_hand(SPN_Span line) {
  if(line.len < 6) {
    LOG_STAT(STAT_ERR_ARGS, "bad hand line size: %zu", line.len);
    return (Hand){0};
  }

  Hand hand = {0};

  for(size_t i = 0; i < HAND_SIZE; i++) {
    const char c  = *(const char *)SPN_get(line, i);
    hand.cards[i] = char_to_card(c);
  }

  SPN_Span bid_span = SPN_subspan(line, 6, (line.len - 6));

  if(sscanf(bid_span.begin, "%zu", &hand.bid) != 1) {
    LOG_STAT(STAT_ERR_ARGS, "no bid found in input: '%.*s'", line.len, line.begin);
  }

  return hand;
}

typedef struct HandHistogram {
  size_t occurrences[NUM_CARD_TYPES];
} HandHistogram;

static HandHistogram get_hand_histogram(Hand hand) {
  HandHistogram hist = {0};
  for(size_t i = 0; i < HAND_SIZE; i++) { hist.occurrences[hand.cards[i]]++; }

  return hist;
}

static HandType get_hand_type_part1(Hand hand) {
  const HandHistogram hist = get_hand_histogram(hand);

  size_t num_sets     = 0;
  size_t max_set_size = 0;

  for(size_t i = 0; i < NUM_CARD_TYPES; i++) {
    const size_t set_size = hist.occurrences[i];
    if(set_size >= 2) num_sets++;
    max_set_size = (set_size > max_set_size) ? set_size : max_set_size;
  }

  return (HandType)((max_set_size == 5)                        ? FIVE_OF_A_KIND
                    : (max_set_size == 4)                      ? FOUR_OF_A_KIND
                    : ((max_set_size == 3) && (num_sets == 2)) ? FULL_HOUSE
                    : (max_set_size == 3)                      ? THREE_OF_A_KIND
                    : ((max_set_size == 2) && (num_sets == 2)) ? TWO_PAIR
                    : (max_set_size == 2)                      ? ONE_PAIR
                                                               : HIGH_CARD);
}

static bool contains_jokers(Hand hand) {
  for(size_t i = 0; i < HAND_SIZE; i++) {
    if(hand.cards[i] == JOKER) return true;
  }
  return false;
}

HandType get_hand_type(Hand hand) {
  if(contains_jokers(hand)) {
    // try replacing jokers by all types of cards, and take the highest resulting hand type
    HandType max_hand_type = HIGH_CARD;
    for(Card replacement = TWO; replacement < NUM_CARD_TYPES; replacement++) {
      if(replacement == JACK) continue; // no JACK anymore

      Hand modified_hand = hand;
      for(size_t i = 0; i < HAND_SIZE; i++) {
        if(modified_hand.cards[i] == JOKER) modified_hand.cards[i] = replacement;
      }

      HandType type = get_hand_type_part1(modified_hand);
      max_hand_type = (type > max_hand_type) ? type : max_hand_type;
    }

    return max_hand_type;
  }

  return get_hand_type_part1(hand);
}

int compare_hands(Hand a, Hand b) {
  const HandType a_type = get_hand_type(a);
  const HandType b_type = get_hand_type(b);
  if(a_type > b_type) return 1;
  if(a_type < b_type) return -1;

  for(size_t i = 0; i < HAND_SIZE; i++) {
    if(a.cards[i] > b.cards[i]) return 1;
    if(a.cards[i] < b.cards[i]) return -1;
  }

  return 0;
}

int compare_hands_for_qsort(const void * a, const void * b) {
  return compare_hands(*((const Hand *)a), *((const Hand *)b));
}

STAT_Val sort_hands_by_rank(SPN_MutSpan hands) {
  CHECK(!SPN_is_empty(SPN_mut_to_const(hands)));
  CHECK(hands.element_size == sizeof(Hand));

  qsort(hands.begin, hands.len, hands.element_size, compare_hands_for_qsort);

  return OK;
}

STAT_Val parse_hands_part1(const DAR_DArray * lines, DAR_DArray * hands) {
  CHECK(lines != NULL);
  CHECK(DAR_is_initialized(lines));
  CHECK(!DAR_is_empty(lines));
  CHECK(lines->element_size == sizeof(DAR_DArray));
  CHECK(hands != NULL);
  CHECK(DAR_is_initialized(hands));
  CHECK(DAR_is_empty(hands));
  CHECK(hands->element_size == sizeof(Hand));

  for(const DAR_DArray * line = DAR_first(lines); line != DAR_end(lines); line++) {
    Hand hand = parse_hand(DAR_to_span(line));
    TRY(DAR_push_back(hands, &hand));
  }

  return OK;
}

STAT_Val parse_hands_part2(const DAR_DArray * lines, DAR_DArray * hands) {
  TRY(parse_hands_part1(lines, hands));

  for(Hand * hand = DAR_first(hands); hand != DAR_end(hands); hand++) {
    for(size_t i = 0; i < HAND_SIZE; i++) {
      if(hand->cards[i] == JACK) hand->cards[i] = JOKER;
    }
  }

  return OK;
}

STAT_Val get_total_winnings(SPN_MutSpan hands, size_t * out) {
  CHECK(!SPN_is_empty(SPN_mut_to_const(hands)));
  CHECK(out != NULL);

  TRY(sort_hands_by_rank(hands));

  size_t total_winnings = 0;

  size_t rank = 1;
  for(const Hand * hand = SPN_first(hands); hand != SPN_end(hands); hand++, rank++) {
    total_winnings += (rank * hand->bid);
  }

  *out = total_winnings;

  return OK;
}