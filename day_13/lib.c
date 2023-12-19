#include <cfac/log.h>

#include "common.h"
#include "lib.h"

#include <stdio.h>

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

    (*width)++;
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

  for(size_t out_row_idx = 0; out_row_idx < out->rows.size; out_row_idx++) {
    size_t * out_row = DAR_get(&out->rows, out_row_idx);

    const size_t in_bit_idx = (out->rows.size - out_row_idx - 1);

    for(size_t in_row_idx = 0; in_row_idx < in->rows.size; in_row_idx++) {
      const size_t * in_row       = DAR_get(&in->rows, in_row_idx);
      const size_t   relevant_bit = (*in_row) & (1 << in_bit_idx);

      *out_row <<= 1;
      *out_row |= (relevant_bit ? 1 : 0);
    }
  }

  return OK;
}

STAT_Val find_mirror(const Pattern * in, bool * has_mirror, size_t * mirror_position) {
  CHECK(in != NULL);
  CHECK(DAR_is_initialized(&in->rows));
  CHECK(in->rows.size > 1);
  CHECK(mirror_position != NULL);

  *mirror_position = 0;
  *has_mirror      = false;

  size_t row_idx = 0;

  while((row_idx < in->rows.size) && !(*has_mirror)) {
    bool maybe_has_mirror = false;
    for(size_t i = row_idx; i < (in->rows.size - 1); i++) {
      const size_t * row  = DAR_get(&in->rows, i);
      const size_t * next = DAR_get(&in->rows, i + 1);

      if(*row == *next) {
        row_idx          = i;
        maybe_has_mirror = true;
        break;
      }
    }
    if(!maybe_has_mirror) break;

    for(size_t i = 0; (i <= row_idx) && maybe_has_mirror; i++) {
      const size_t * row = DAR_get(&in->rows, row_idx - i);

      const size_t opposite_idx = row_idx + i + 1;

      if(opposite_idx < in->rows.size) {
        const size_t * other = DAR_get(&in->rows, opposite_idx);

        if(*row != *other) maybe_has_mirror = false;
      }
    }

    if(maybe_has_mirror) {
      *has_mirror      = true;
      *mirror_position = row_idx + 1;
    }
    row_idx++;
  }

  return OK;
}

STAT_Val print_pattern(const Pattern * pattern) {
  CHECK(pattern != NULL);
  CHECK(pattern->width > 0);
  CHECK(DAR_is_initialized(&pattern->rows));
  CHECK(pattern->rows.size > 0);

  for(const size_t * row = DAR_first(&pattern->rows); row != DAR_end(&pattern->rows); row++) {
    for(int i = pattern->width - 1; i >= 0; i--) putc(((*row) & (1 << i)) ? '#' : '.', stdout);
    putc('\n', stdout);
  }

  return OK;
}