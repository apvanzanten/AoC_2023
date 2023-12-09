#ifndef lib_h
#define lib_h

#include <cfac/darray.h>
#include <cfac/span.h>
#include <cfac/stat.h>

#include "common.h"

typedef enum Card { TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE, NUM_CARD_TYPES } Card;

#define HAND_SIZE 5

typedef struct Hand {
  Card   cards[HAND_SIZE];
  size_t bid;
} Hand;

typedef enum HandType {
  HIGH_CARD,
  ONE_PAIR,
  TWO_PAIR,
  THREE_OF_A_KIND,
  FULL_HOUSE,
  FOUR_OF_A_KIND,
  FIVE_OF_A_KIND,
} HandType;

Hand parse_hand(SPN_Span line);

HandType get_hand_type(Hand hand);

int compare_hands(Hand a, Hand b);

static inline bool hand_equals(Hand a, Hand b) { return compare_hands(a, b) == 0; }

STAT_Val sort_hands_by_rank(SPN_MutSpan hands);

STAT_Val parse_hands(const DAR_DArray * lines, DAR_DArray * hands);

STAT_Val get_total_winnings(SPN_MutSpan hands, size_t * out);

#endif