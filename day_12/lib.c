#include <cfac/log.h>
#include <cfac/span.h>

#include <stdio.h>

#include "common.h"
#include "lib.h"

void print_binary(size_t n, size_t num_bits_to_print) {
  putc('0', stdout);
  putc('b', stdout);

  char s[64 + 1] = {0};

  for(size_t bit_idx = 0; bit_idx < 64; bit_idx++) {
    size_t mag = (63 - bit_idx);
    s[bit_idx] = (n & ((size_t)1 << mag)) ? '1' : '0';
  }

  printf("%s", &s[(64 - num_bits_to_print)]);
}

static STAT_Val parse_conditions(SPN_Span str, Record * record) {
  CHECK(record != NULL);
  CHECK(str.len >= 1);

  record->operational_bits = 0;
  record->damaged_bits     = 0;
  record->unknown_bits     = 0;

  size_t num_conditions = 0;

  for(size_t i = 0; i < str.len; i++) {
    const char c = *(const char *)SPN_get(str, i);
    if(c == '.' || c == '#' || c == '?') {
      record->operational_bits = (record->operational_bits << 1);
      record->damaged_bits     = (record->damaged_bits << 1);
      record->unknown_bits     = (record->unknown_bits << 1);

      switch(c) {
      case '.': record->operational_bits = (record->operational_bits | 1); break;
      case '#': record->damaged_bits = (record->damaged_bits | 1); break;
      case '?': record->unknown_bits = (record->unknown_bits | 1); break;
      }

      num_conditions++;
    }
    if(num_conditions > 63) return LOG_STAT(STAT_ERR_READ, "too many conditions!");
  }

  return OK;
}

static STAT_Val parse_groups(SPN_Span str, DAR_DArray * groups) {
  CHECK(groups != NULL);
  CHECK(DAR_is_initialized(groups));
  CHECK(groups->element_size == sizeof(size_t));
  CHECK(DAR_is_empty(groups));
  CHECK(str.len >= 1);

  const char delim = ',';
  while(str.len > 0) {
    const char * p = str.begin;
    if(*p == delim) continue;

    size_t n = 0;
    CHECK(sscanf(p, "%zu", &n) == 1);
    TRY(DAR_push_back(groups, &n));

    size_t start_of_next_num = 0;
    if(SPN_find(str, &delim, &start_of_next_num) != STAT_OK) break; // assume end is here
    start_of_next_num++;
    str = SPN_subspan(str, start_of_next_num, (str.len - start_of_next_num));
  }

  return OK;
}

static STAT_Val init_record(Record * record) {
  CHECK(record != NULL);

  *record = (Record){0};

  TRY(DAR_create(&record->groups, sizeof(size_t)));

  return OK;
}

STAT_Val parse_records(const DAR_DArray * lines, DAR_DArray * records /*contains Record*/) {
  CHECK(lines != NULL);
  CHECK(DAR_is_initialized(lines));
  CHECK(lines->element_size == sizeof(DAR_DArray));
  CHECK(!DAR_is_empty(lines));
  CHECK(records != NULL);
  CHECK(DAR_is_initialized(records));
  CHECK(records->element_size == sizeof(Record));
  CHECK(DAR_is_empty(records));

  for(const DAR_DArray * line = DAR_first(lines); line != DAR_end(lines); line++) {
    size_t     space_idx = 0;
    const char space     = ' ';
    CHECK(SPN_find(DAR_to_span(line), &space, &space_idx) == STAT_OK);

    Record record = {0};
    TRY(init_record(&record));

    TRY(parse_conditions(SPN_subspan(DAR_to_span(line), 0, space_idx), &record));
    TRY(parse_groups(SPN_subspan(DAR_to_span(line), space_idx + 1, line->size - (space_idx + 1)), &record.groups));
    TRY(DAR_push_back(records, &record));
  }

  return OK;
}

static STAT_Val destroy_record(Record * record) {
  CHECK(record != NULL);

  TRY(DAR_destroy(&record->groups));

  *record = (Record){0};

  return OK;
}

STAT_Val destroy_records(DAR_DArray * records) {
  CHECK(records != NULL);

  for(Record * record = DAR_first(records); record != DAR_end(records); record++) { TRY(destroy_record(record)); }

  return OK;
}

static uint8_t popcount(size_t n) {
  uint8_t c = 0;

  while(n != 0) {
    n >>= 1;
    c++;
  }

  return c;
}

static STAT_Val print_groups(SPN_Span groups) {
  for(const size_t * group = SPN_first(groups); group != SPN_end(groups); group++) { printf("%zu,", *group); }
  return OK;
}

static void print_record_conditions_by_bits(size_t operational_bits, size_t damaged_bits, size_t unknown_bits) {
  size_t count = popcount(operational_bits | damaged_bits | unknown_bits);
  for(size_t i = 0; i < count; i++) {
    size_t mask = (1 << ((count - i) - 1));
    if(operational_bits & mask) {
      putc('.', stdout);
    } else if(damaged_bits & mask) {
      putc('#', stdout);
    } else if(unknown_bits & mask) {
      putc('?', stdout);
    } else {
      putc('_', stdout);
    }
  }
}

static STAT_Val print_record(const Record * record) {
  CHECK(record != NULL);

  print_record_conditions_by_bits(record->operational_bits, record->damaged_bits, record->unknown_bits);

  putc(' ', stdout);

  TRY(print_groups(DAR_to_span(&record->groups)));

  putc('\n', stdout);

  return OK;
}

STAT_Val print_records(const DAR_DArray * records) {
  CHECK(records != NULL);

  for(const Record * record = DAR_first(records); record != DAR_end(records); record++) { TRY(print_record(record)); }
  return OK;
}

size_t get_num_possibilities_rec(size_t   operational_bits,
                                 size_t   damaged_bits,
                                 size_t   unknown_bits,
                                 size_t   start_idx,
                                 SPN_Span groups) {
  if(groups.len == 0) return 1;
  const size_t group_size = *(const size_t *)SPN_first(groups);
  const size_t bits_left  = (64 - start_idx);
  if(bits_left < group_size) return 0;

  const size_t group_bits = ((1 << group_size) - 1);

  size_t num_possibilities = 0;
  for(size_t i = start_idx; i < (64 - (group_size - 1)); i++) {
    size_t group_shift = (64 - i - group_size);
    size_t group_mask  = group_bits << group_shift;
    size_t gap_mask    = ((group_mask >> 1) ^ group_mask) & ~group_mask; // wow :o

    size_t possibly_damaged_bits     = (damaged_bits | unknown_bits);
    size_t possibly_operational_bits = (operational_bits | unknown_bits);

    const bool group_fits          = ((possibly_damaged_bits & group_mask) == group_mask);
    const bool skipping_damage     = (damaged_bits & ~group_mask) > group_mask; // wooh! :D
    const bool has_gap_before_next = !gap_mask || (possibly_operational_bits & gap_mask);

    if(group_fits && has_gap_before_next && !skipping_damage) {
      if(groups.len == 1) {
        int    remaining_bits = 64 - (i + group_size);
        size_t remainder_mask = (remaining_bits == 64) ? SIZE_MAX : ((((size_t)1) << remaining_bits) - 1);
        if(damaged_bits & remainder_mask) {
          // we still have damage remaining, this is not a possibility!
          continue;
        } else {
          num_possibilities++;
        }
      } else {
        size_t new_start_idx = i + group_size + 1;
        if(new_start_idx > 64) break; // ran out of bits!

        int    remaining_bits = 64 - new_start_idx;
        size_t remainder_mask = (remaining_bits == 64) ? SIZE_MAX : ((((size_t)1) << remaining_bits) - 1);

        size_t remaining_operational_bits = operational_bits & remainder_mask;
        size_t remaining_damaged_bits     = damaged_bits & remainder_mask;
        size_t remaining_unknown_bits     = unknown_bits & remainder_mask;

        num_possibilities += get_num_possibilities_rec(remaining_operational_bits,
                                                       remaining_damaged_bits,
                                                       remaining_unknown_bits,
                                                       new_start_idx,
                                                       SPN_subspan(groups, 1, groups.len - 1));
      }
    }
    if(skipping_damage) break;
  }

  return num_possibilities;
}

STAT_Val get_num_possibilities_for_record(const Record * record, size_t * num_possibilities) {
  CHECK(record != NULL);
  CHECK(num_possibilities != NULL);

  *num_possibilities = get_num_possibilities_rec(record->operational_bits,
                                                 record->damaged_bits,
                                                 record->unknown_bits,
                                                 24,
                                                 DAR_to_span(&record->groups));

  return OK;
}

STAT_Val get_num_possibilities_for_all_records(const DAR_DArray * records, size_t * num_possibilities) {
  CHECK(records != NULL);
  CHECK(DAR_is_initialized(records));
  CHECK(records->element_size == sizeof(Record));
  CHECK(num_possibilities != NULL);

  *num_possibilities = 0;
  for(const Record * record = DAR_first(records); record != DAR_end(records); record++) {
    size_t num_possibilities_for_record = 0;
    TRY(get_num_possibilities_for_record(record, &num_possibilities_for_record));
    (*num_possibilities) += num_possibilities_for_record;
  }

  return OK;
}