#include <cfac/log.h>
#include <cfac/span.h>

#include "lib.h"

#include "common.h"

#include <cfac/darray.h>

#include <stdio.h>

#define OK STAT_OK

static const char * g_color_strings[] = {
    [RED]   = "red",
    [GREEN] = "green",
    [BLUE]  = "blue",
};

static STAT_Val get_game_id(SPN_Span line, int * out) {
  CHECK(line.len > 0);
  CHECK(out != NULL);

  size_t pos_of_game = SIZE_MAX;
  TRY(SPN_find_subspan(line, SPN_from_cstr("Game "), &pos_of_game));
  CHECK(pos_of_game == 0);

  CHECK(sscanf(line.begin, "Game %d:", out) == 1);

  return OK;
}

static STAT_Val split_by_delim(SPN_Span span, SPN_Span delim, DAR_DArray * spans) {
  CHECK(delim.len > 0);
  CHECK(spans != NULL);
  CHECK(DAR_is_initialized(spans));

  while(span.len != 0) {
    size_t   delim_idx = 0;
    STAT_Val find_res  = SPN_find_subspan(span, delim, &delim_idx);

    CHECK(STAT_is_OK(find_res));

    if(find_res == STAT_OK) {
      SPN_Span subsp = SPN_subspan(span, 0, delim_idx); // take subspan from 0 to start of delim
      TRY(DAR_push_back(spans, &subsp));
      span = SPN_subspan(span, subsp.len + delim.len, span.len);
    } else if(find_res == STAT_OK_NOT_FOUND) {
      // delim not found, add current span fully (if it is not empty)
      if(span.len > 0) { TRY(DAR_push_back(spans, &span)); }
      break;
    } else {
      break;
    }
  }

  return OK;
}

static STAT_Val trim_whitespace(SPN_Span * span) {
  CHECK(span != NULL);

  while(span->len != 0 && *(char *)SPN_first(*span) == ' ') *span = SPN_subspan(*span, 1, span->len - 1);
  while(span->len != 0 && *(char *)SPN_last(*span) == ' ') *span = SPN_subspan(*span, 0, span->len - 1);

  return OK;
}

static STAT_Val get_color_entry(SPN_Span color_entry_span, int * num, Color * color) {
  CHECK(color_entry_span.len > 1);
  CHECK(num != NULL);
  CHECK(color != NULL);

  TRY(trim_whitespace(&color_entry_span));

  DAR_DArray count_and_color = {0};
  TRY(DAR_create(&count_and_color, sizeof(SPN_Span)));

  TRY(split_by_delim(color_entry_span, SPN_from_cstr(" "), &count_and_color));

  CHECK(sscanf(((SPN_Span *)DAR_first(&count_and_color))->begin, "%d", num) == 1);

  bool found = false;
  for(Color c = COLOR_FIRST; c != COLOR_END; c++) {
    size_t idx = SIZE_MAX;
    TRY(SPN_find_subspan(*(SPN_Span *)DAR_last(&count_and_color), SPN_from_cstr(g_color_strings[c]), &idx));
    if(idx == 0) {
      found  = true;
      *color = c;
    }
  }
  if(!found) return LOG_STAT(STAT_ERR_NOT_FOUND, "failed to find color");

  TRY(DAR_destroy(&count_and_color));

  return OK;
}

STAT_Val get_game_result_from_line(SPN_Span line, GameResult * out) {
  CHECK(line.len > 0);
  CHECK(out != NULL);
  *out = (GameResult){0};

  DAR_DArray game_and_sets = {0};

  TRY(DAR_create(&game_and_sets, sizeof(SPN_Span)));

  TRY(split_by_delim(line, SPN_from_cstr(":"), &game_and_sets));
  TRY(get_game_id(*(SPN_Span *)DAR_first(&game_and_sets), &(out->game_id)));

  if(game_and_sets.size > 1) {
    DAR_DArray sets = {0};
    TRY(DAR_create(&sets, sizeof(SPN_Span)));

    TRY(split_by_delim(*(SPN_Span *)DAR_last(&game_and_sets), SPN_from_cstr(";"), &sets));

    for(SPN_Span * sp = DAR_first(&sets); sp != DAR_end(&sets); sp++) {
      DAR_DArray color_entry_spans = {0};
      TRY(DAR_create(&color_entry_spans, sizeof(SPN_Span)));

      TRY(split_by_delim(*sp, SPN_from_cstr(","), &color_entry_spans));

      for(SPN_Span * entry_span = DAR_first(&color_entry_spans); entry_span != DAR_end(&color_entry_spans);
          entry_span++) {
        Color color = RED;
        int   count = 0;
        TRY(get_color_entry(*entry_span, &count, &color));

        out->min_color_occurrences[color] =
            (out->min_color_occurrences[color] < count) ? count : out->min_color_occurrences[color];
      }

      TRY(DAR_destroy(&color_entry_spans));
    }

    TRY(DAR_destroy(&sets));
  }

  TRY(DAR_destroy(&game_and_sets));

  return OK;
}
