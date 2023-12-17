#include <cfac/log.h>

#include "common.h"
#include "lib.h"

static STAT_Val init_pattern(Pattern * pattern) {
  CHECK(pattern != NULL);

  *pattern = (Pattern){0};

  TRY(DAR_create(&pattern->rows, sizeof(size_t)));

  return OK;
}

static STAT_Val parse_row(SPN_Span line, size_t * row, size_t * width) {
  CHECK(!SPN_is_empty(line));
  CHECK(line.element_size == sizeof(char));
  CHECK(row != NULL);
  CHECK(width != NULL);

  *width = 0;
  *row   = 0;

  for(const char * c = SPN_first(line); c != SPN_end(line); c++) {
    if(!(*c == '#' || *c == '.')) break;

    *row <<= 1;
    *row |= (*c == '#') ? 1 : 0;

    width++;
  }

  return OK;
}

STAT_Val parse_pattern(SPN_Span lines, Pattern * out) {
  CHECK(!SPN_is_empty(lines));
  CHECK(lines.element_size == sizeof(DAR_DArray));
  CHECK(out != NULL);

  TRY(init_pattern(out));

  for(const DAR_DArray * line = SPN_first(lines); line != SPN_end(lines); line++) {
    size_t width = 0;
    size_t row   = 0;

    TRY(parse_row(DAR_to_span(line), &row, &width));

    if(out->width == 0) out->width = width;
    CHECK(out->width == width);

    TRY(DAR_push_back(&out->rows, &row));
  }

  return OK;
}

STAT_Val destroy_pattern(Pattern * pattern) {
  CHECK(pattern != NULL);

  TRY(DAR_destroy(&pattern->rows));
  *pattern = (Pattern){0};

  return OK;
}

STAT_Val transpose_pattern(const Pattern * in, Pattern * out) {
  CHECK(in != NULL);
  CHECK(out != NULL);

  TRY(init_pattern(out));

  out->width        = in->rows.size;
  const size_t zero = 0;
  TRY(DAR_resize_with_value(&out->rows, in->width, &zero));

  for(size_t y = 0; y < out->rows.size; y++) {
    size_t * row = DAR_get(&out->rows, y);
    for(size_t x = 0; x < in->rows.size; x++) {
      const size_t * in_row       = DAR_get(&in->rows, x);
      const size_t   relevant_bit = (*in_row) & (1 << (in->width - y));
      *row |= (relevant_bit) ? 1 : 0;
      *row <<= 1;
    }
  }

  return OK;
}