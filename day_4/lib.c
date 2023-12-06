#include <cfac/log.h>

#include "common.h"
#include "lib.h"

#include <cfac/darray.h>
#include <cfac/list.h>

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

static STAT_Val calculate_win_count(const Card * card, size_t * win_count) {
  CHECK(card != NULL);
  CHECK(win_count != NULL);

  *win_count = 0;

  for(const int * num_p = DAR_first(&card->numbers); num_p != DAR_end(&card->numbers); num_p++) {
    for(const int * win_num_p = DAR_first(&card->winning_numbers); win_num_p != DAR_end(&card->winning_numbers);
        win_num_p++) {
      if(*num_p == *win_num_p) (*win_count)++;
    }
  }

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

  size_t win_count = 0;

  TRY(get_card_win_count(card_line, &win_count));

  *score = (win_count > 0) ? (1 << (win_count - 1)) : 0;

  return OK;
}

STAT_Val get_card_win_count(SPN_Span card_line, size_t * win_count) {
  CHECK(card_line.len > 1);
  CHECK(win_count != NULL);

  Card card = {0};

  TRY(parse_card(card_line, &card));

  TRY(calculate_win_count(&card, win_count));

  TRY(destroy_card(&card));

  return OK;
}

STAT_Val get_total_number_of_cards(const DAR_DArray * lines, size_t * num_of_cards) {
  CHECK(lines != NULL);
  CHECK(lines->element_size == sizeof(DAR_DArray));
  CHECK(num_of_cards != NULL);

  DAR_DArray win_counts          = {0};
  LST_List   to_be_counted_cards = {0};
  TRY(DAR_create(&win_counts, sizeof(size_t)));
  TRY(DAR_reserve(&win_counts, lines->size));
  TRY(LST_create(&to_be_counted_cards, sizeof(size_t)));

  *num_of_cards = 0;

  // first get all the win counts, as well as counting each card once

  for(size_t idx = 0; idx < lines->size; idx++) {
    const DAR_DArray * line = DAR_get(lines, idx);

    size_t win_count = 0;
    TRY(get_card_win_count(DAR_to_span(line), &win_count));

    TRY(DAR_push_back(&win_counts, &win_count));
    TRY(LST_insert(&to_be_counted_cards, LST_end(&to_be_counted_cards), &idx, NULL));
    (*num_of_cards)++;
  }

  // keep going through list of to-be-counted cards, adding copies to the end, until it is empty
  while(!LST_is_empty(&to_be_counted_cards)) {
    const size_t card_idx = *(size_t *)LST_data(LST_first(&to_be_counted_cards));
    TRY(LST_remove(LST_first(&to_be_counted_cards)));

    const size_t win_count = *(const int *)DAR_get(&win_counts, card_idx);
    for(size_t i = 0; i < win_count; i++) {
      const size_t copy_idx = card_idx + i + 1;

      if(copy_idx < win_counts.size) {
        TRY(LST_insert(&to_be_counted_cards, LST_end(&to_be_counted_cards), &copy_idx, NULL));
        (*num_of_cards)++;
      }
    }
  }

  TRY(LST_destroy(&to_be_counted_cards));
  TRY(DAR_destroy(&win_counts));

  return OK;
}