#include <cfac/log.h>

#include "lib.h"

#include "common.h"

#include <stdio.h>

typedef struct Index {
  int x, y;
} Index;

typedef struct IndexedNumber {
  int x, y, number;
} IndexedNumber;

static bool is_num(char c) { return (c >= '0') && (c <= '9'); }
static bool is_symbol(char c) { return !is_num(c) && (c != '.') && (c != '\n'); }

static STAT_Val get_symbols(SPN_Span schematic, DAR_DArray * symbol_indices) {
  CHECK((int)schematic.len > 0);
  CHECK(schematic.element_size == sizeof(SPN_Span));
  CHECK(symbol_indices != NULL);
  CHECK(DAR_is_initialized(symbol_indices));
  CHECK(symbol_indices->element_size == sizeof(Index));

  for(int y = 0; y < (int)schematic.len; y++) {
    SPN_Span line = *(SPN_Span *)SPN_get(schematic, y);

    for(int x = 0; x < (int)line.len; x++) {
      char c = *(const char *)SPN_get(line, x);
      if(is_symbol(c)) {
        Index idx = {x, y};
        TRY(DAR_push_back(symbol_indices, &idx));
      }
    }
  }

  return OK;
}

static STAT_Val get_full_number(SPN_Span line, int pos_x, int * number, int * start_x, int * end_x) {
  CHECK(pos_x < (int)line.len);
  CHECK(number != NULL);
  CHECK(end_x != NULL);

  // expand left to find start of number, then interpret

  *start_x = pos_x;
  *end_x   = pos_x;
  while(*start_x >= 0 && is_num(*(const char *)SPN_get(line, *start_x))) { (*start_x)--; }
  (*start_x)++; // reached to just-before number, increment start once

  while(*end_x < (int)line.len && is_num(*(const char *)SPN_get(line, *end_x))) { (*end_x)++; }

  CHECK(sscanf(SPN_get(line, *start_x), "%d", number) == 1);

  return OK;
}

static STAT_Val get_adjacent_numbers(SPN_Span schematic, Index symbol_index, DAR_DArray * numbers) {
  CHECK(symbol_index.y < (int)schematic.len);
  CHECK(numbers != NULL);

  const int height = schematic.len;
  const int width  = (*(const SPN_Span *)SPN_first(schematic)).len;

  CHECK(height >= 1);
  CHECK(width >= 1);

  const int start_y = clamp_int(symbol_index.y - 1, 0, height - 1);
  const int end_y   = clamp_int(symbol_index.y + 1, 0, height - 1);
  const int start_x = clamp_int(symbol_index.x - 1, 0, width - 1);
  const int end_x   = clamp_int(symbol_index.x + 1, 0, width - 1);

  for(int y = start_y; y <= end_y; y++) {
    SPN_Span line = *(const SPN_Span *)SPN_get(schematic, y);

    for(int x = start_x; x <= end_x; x++) {
      char c = *(const char *)SPN_get(line, x);
      if(is_num(c)) {
        int number  = 0;
        int end_x   = 0;
        int start_of_num_x = 0;
        TRY(get_full_number(line, x, &number, &start_of_num_x, &end_x));

        IndexedNumber idx_num = {.x = start_of_num_x, .y = y, .number = number};
        TRY(DAR_push_back(numbers, &idx_num));

        x = end_x; // may end inner loop
      }
    }
  }

  return OK;
}

STAT_Val get_numbers_from_schematic(SPN_Span schematic, DAR_DArray * numbers) {
  CHECK((int)schematic.len > 0);
  CHECK(schematic.element_size == sizeof(SPN_Span));
  CHECK(numbers != NULL);
  CHECK(DAR_is_initialized(numbers));
  CHECK(numbers->element_size == sizeof(int));

  LOG_STAT(STAT_OK, "schematic size: %zu,%zu", schematic.len, (*(const SPN_Span *)SPN_first(schematic)).len);

  DAR_DArray symbol_indices = {0};
  TRY(DAR_create(&symbol_indices, sizeof(Index)));

  DAR_DArray indexed_numbers = {0};
  TRY(DAR_create(&indexed_numbers, sizeof(IndexedNumber)));

  TRY(get_symbols(schematic, &symbol_indices));

  LOG_STAT(STAT_OK, "found %zu symbols", symbol_indices.size);

  for(Index * idx_p = DAR_first(&symbol_indices); idx_p != DAR_end(&symbol_indices); idx_p++) {
    TRY(get_adjacent_numbers(schematic, *idx_p, &indexed_numbers));
  }

  // find one number for each index
  for(int y = 0; y < (int)schematic.len; y++) {
    for(int x = 0; x < (int)(*(const SPN_Span *)SPN_first(schematic)).len; x++) {
      for(const IndexedNumber * n = DAR_first(&indexed_numbers); n != DAR_end(&indexed_numbers); n++) {
        if(n->x == x && n->y == y) {
          TRY(DAR_push_back(numbers, &(n->number)));
          break;
        }
      }
    }
  }

  LOG_STAT(STAT_OK, "indexed_numbers.size: %zu, numbers.size: %zu", indexed_numbers.size, numbers->size);

  TRY(DAR_destroy(&symbol_indices));
  TRY(DAR_destroy(&indexed_numbers));

  return OK;
}