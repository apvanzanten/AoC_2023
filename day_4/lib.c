#include <cfac/log.h>

#include "common.h"
#include "lib.h"

#include <cfac/darray.h>

#include <stdio.h>

typedef struct Card {
  int        id;
  DAR_DArray winning_numbers;
  DAR_DArray numbers;
} Card;

static STAT_Val get_card_id(SPN_Span card_line, int * id) {
  CHECK(id != NULL);

  CHECK(sscanf(card_line.begin, "Card %d:", id) == 1);

  return OK;
}

static STAT_Val get_number_spans(SPN_Span card_line, SPN_Span * winning_number_span, SPN_Span * number_span) {
  CHECK(winning_number_span != NULL);
  CHECK(number_span != NULL);

  const char card_divider       = ':';
  const char number_divider     = '|';
  size_t     card_divider_idx   = 0;
  size_t     number_divider_idx = 0;
  CHECK(SPN_find(card_line, &card_divider, &card_divider_idx) == STAT_OK);     // STAT_OK -> found!
  CHECK(SPN_find(card_line, &number_divider, &number_divider_idx) == STAT_OK); // STAT_OK -> found!

  const size_t start_of_winning_numbers = card_divider_idx + 2;
  const size_t end_of_winning_numbers   = number_divider_idx - 1;
  const size_t start_of_numbers         = number_divider_idx + 2;
  const size_t end_of_numbers           = card_line.len;

  CHECK(end_of_winning_numbers > start_of_winning_numbers);
  CHECK(end_of_numbers > start_of_numbers);

  *winning_number_span =
      SPN_subspan(card_line, start_of_winning_numbers, (end_of_winning_numbers - start_of_winning_numbers));
  *number_span = SPN_subspan(card_line, start_of_numbers, (end_of_numbers - start_of_numbers));

  return OK;
}

static STAT_Val get_numbers(SPN_Span span, DAR_DArray * numbers) {
  CHECK(numbers != NULL);
  CHECK(DAR_is_initialized(numbers));
  CHECK(numbers->element_size == sizeof(int));

  SPN_Span   remaining = span;
  const char delim     = ' ';
  while(remaining.len > 0) {
    // skip over spaces
    while(remaining.len > 0 && (*(const char *)SPN_first(remaining)) == ' ') {
      remaining = SPN_subspan(remaining, 1, remaining.len - 1);
    }

    int n = 0;
    CHECK(sscanf(remaining.begin, "%d", &n) == 1);
    TRY(DAR_push_back(numbers, &n));

    size_t   next_delim_idx = 0;
    STAT_Val find_res       = SPN_find(remaining, &delim, &next_delim_idx);
    if(find_res == STAT_OK) {
      remaining = SPN_subspan(remaining, next_delim_idx + 1, remaining.len - next_delim_idx);
    } else if(find_res == STAT_OK_NOT_FOUND) {
      break;
    } else {
      return LOG_STAT(find_res, "oh no!");
    }
  }

  return OK;
}

static STAT_Val parse_card(SPN_Span card_line, Card * card) {
  CHECK(card != NULL);

  TRY(get_card_id(card_line, &(card->id)));

  SPN_Span winning_number_span = {0};
  SPN_Span number_span         = {0};

  TRY(get_number_spans(card_line, &winning_number_span, &number_span));

  TRY(DAR_create(&card->winning_numbers, sizeof(int)));
  TRY(DAR_create(&card->numbers, sizeof(int)));

  TRY(get_numbers(winning_number_span, &card->winning_numbers));
  TRY(get_numbers(number_span, &card->numbers));

  return OK;
}

static STAT_Val calculate_score(const Card * card, int * score) {
  CHECK(card != NULL);
  CHECK(score != NULL);

  int win_count = 0;

  for(const int * num_p = DAR_first(&card->numbers); num_p != DAR_end(&card->numbers); num_p++) {
    for(const int * win_num_p = DAR_first(&card->winning_numbers); win_num_p != DAR_end(&card->winning_numbers);
        win_num_p++) {
      if(*num_p == *win_num_p) win_count++;
    }
  }

  *score = (win_count > 0) ? (1 << (win_count - 1)) : 0;

  return OK;
}

static STAT_Val destroy_card(Card * card) {
  if(card != NULL) {
    TRY(DAR_destroy(&card->numbers));
    TRY(DAR_destroy(&card->winning_numbers));
  }
  return OK;
}

STAT_Val get_card_score(SPN_Span card_line, int * score) {
  CHECK(card_line.len > 1);
  CHECK(score != NULL);

  Card card = {0};

  TRY(parse_card(card_line, &card));

  TRY(calculate_score(&card, score));

  LOG_STAT(STAT_OK, "card %d has score %d", card.id, *score);

  TRY(destroy_card(&card));

  return OK;
}