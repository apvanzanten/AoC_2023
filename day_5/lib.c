#include <cfac/darray.h>
#include <cfac/log.h>

#include <stdio.h>

#include "common.h"
#include "lib.h"

static STAT_Val init_almanac(Almanac * almanac) {
  CHECK(almanac != NULL);

  TRY(DAR_create(&almanac->seeds, sizeof(size_t)));

  for(MapType type = FIRST_MAP; type <= LAST_MAP; type++) { TRY(DAR_create(&almanac->maps[type], sizeof(MapRange))); }

  return OK;
}

static STAT_Val parse_seeds(const DAR_DArray * line, DAR_DArray * seeds) {
  CHECK(line != NULL);
  CHECK(line->element_size == sizeof(char));
  CHECK(DAR_is_initialized(line));
  CHECK(!DAR_is_empty(line));
  CHECK(seeds != NULL);
  CHECK(DAR_is_initialized(seeds));
  CHECK(DAR_is_empty(seeds));
  CHECK(seeds->element_size == sizeof(size_t));

  CHECK(SPN_contains_subspan(DAR_to_span(line), SPN_from_cstr("seeds: ")));

  SPN_Span   remaining = DAR_to_span(line);
  const char delim     = ' ';
  while(remaining.len > 0) {
    size_t next_delim_idx = 0;

    const STAT_Val find_st = SPN_find(remaining, &delim, &next_delim_idx);
    CHECK(STAT_is_OK(find_st));
    if(find_st != STAT_OK) break; // no more delim found, we must be finished
    CHECK(find_st == STAT_OK);

    remaining = SPN_subspan(remaining, next_delim_idx + 1, remaining.len - (next_delim_idx + 1));

    size_t seed = 0;
    if(sscanf(remaining.begin, "%zu", &seed) != 1) break; // can't find another number, probably means we're finished

    TRY(DAR_push_back(seeds, &seed));
  }

  return OK;
}

static STAT_Val find_line_containing_subspan(SPN_Span lines_span, SPN_Span subspan, size_t * idx) {
  CHECK(idx != NULL);
  CHECK(lines_span.len > 0);
  CHECK(subspan.len > 0);
  CHECK(subspan.begin != NULL);

  for(size_t i = 0; i < lines_span.len; i++) {
    SPN_Span line_span = DAR_to_span(SPN_get(lines_span, i));
    CHECK(line_span.begin != NULL);
    CHECK(line_span.len > 0);

    if(SPN_contains_subspan(line_span, subspan)) {
      *idx = i;
      return OK;
    }
  }

  return STAT_OK_NOT_FOUND;
}

static STAT_Val find_empty_line(SPN_Span lines_span, size_t * idx) {
  CHECK(idx != NULL);
  CHECK(lines_span.len > 0);

  for(size_t i = 0; i < lines_span.len; i++) {
    SPN_Span line_span = DAR_to_span(SPN_get(lines_span, i));
    if(line_span.len == 0 || (*(const char *)SPN_first(line_span)) == '\n') {
      *idx = i;
      return OK;
    }
  }

  return STAT_OK_NOT_FOUND;
}

static STAT_Val parse_map_range(SPN_Span line_span, MapRange * range) {
  CHECK(line_span.len > 0);
  CHECK(range != NULL);

  const char delim = ' ';

  CHECK(sscanf(line_span.begin, "%zu", &range->dst_start) == 1);

  size_t idx = 0;

  CHECK(SPN_find(line_span, &delim, &idx) == OK);
  line_span = SPN_subspan(line_span, idx + 1, line_span.len - (idx + 1));

  CHECK(sscanf(line_span.begin, "%zu", &range->src_start) == 1);

  CHECK(SPN_find(line_span, &delim, &idx) == OK);
  line_span = SPN_subspan(line_span, idx + 1, line_span.len - (idx + 1));

  CHECK(sscanf(line_span.begin, "%zu", &range->length) == 1);

  return OK;
}

static STAT_Val get_maps_span(SPN_Span lines_span, MapType map_type, SPN_Span * out) {
  CHECK(lines_span.len > 0);
  CHECK(out != NULL);

  const char * map_name = map_type_to_str(map_type);

  size_t header_line_idx = 0;
  CHECK(find_line_containing_subspan(lines_span, SPN_from_cstr(map_name), &header_line_idx) == OK);

  size_t         end_idx = 0;
  const STAT_Val find_empty_line_st =
      find_empty_line(SPN_subspan(lines_span, header_line_idx, (lines_span.len - header_line_idx)), &end_idx);

  CHECK(STAT_is_OK(find_empty_line_st));

  if(find_empty_line_st == STAT_OK) {
    end_idx += header_line_idx; // we started looking at header_line_idx, so we need to offset
  } else if(find_empty_line_st == STAT_OK_NOT_FOUND) {
    end_idx = lines_span.len; // no empty line found, assume we're at the end of the file
  }

  *out = SPN_subspan(lines_span, header_line_idx + 1, (end_idx - (header_line_idx + 1)));

  return OK;
}

static STAT_Val parse_maps(const DAR_DArray * lines, Almanac * almanac) {
  CHECK(lines != NULL);
  CHECK(almanac != NULL);
  CHECK(lines->element_size == sizeof(DAR_DArray));

  const SPN_Span lines_span = DAR_to_span(lines);

  for(MapType map_type = FIRST_MAP; map_type <= LAST_MAP; map_type++) {
    SPN_Span maps_span = {0};

    TRY(get_maps_span(lines_span, map_type, &maps_span));

    for(const DAR_DArray * line = SPN_first(maps_span); line != SPN_end(maps_span); line++) {
      MapRange range = {0};

      TRY(parse_map_range(DAR_to_span(line), &range));
      TRY(DAR_push_back(&almanac->maps[map_type], &range));
    }
  }

  return OK;
}

STAT_Val parse_almanac(const DAR_DArray * lines, Almanac * out) {
  CHECK(lines != NULL);
  CHECK(lines->element_size == sizeof(DAR_DArray));
  CHECK(DAR_is_initialized(lines));
  CHECK(!DAR_is_empty(lines));
  CHECK(out != NULL);

  TRY(init_almanac(out));
  TRY(parse_seeds(DAR_first(lines), &out->seeds));
  TRY(parse_maps(lines, out));

  return OK;
}

STAT_Val destroy_almanac(Almanac * almanac) {
  CHECK(almanac != NULL);

  TRY(DAR_destroy(&almanac->seeds));

  for(MapType type = FIRST_MAP; type <= LAST_MAP; type++) { TRY(DAR_destroy(&almanac->maps[type])); }

  return OK;
}

static STAT_Val map_number(size_t num, const DAR_DArray * map_ranges, size_t * out) {
  CHECK(map_ranges != NULL);
  CHECK(map_ranges->element_size == sizeof(MapRange));
  CHECK(out != NULL);

  for(const MapRange * range = DAR_first(map_ranges); range != DAR_end(map_ranges); range++) {
    if(num >= range->src_start && num < (range->src_start + range->length)) {
      *out = range->dst_start + (num - range->src_start);
      return OK;
    }
  }

  // not in any of the map ranges, that means in == out
  *out = num;

  return OK;
}

STAT_Val find_lowest_location_number(const Almanac * almanac, size_t * out) {
  CHECK(almanac != NULL);
  CHECK(out != NULL);

  const DAR_DArray * seeds = &almanac->seeds;

  size_t lowest = SIZE_MAX;

  for(const size_t * seed_p = DAR_first(seeds); seed_p != DAR_end(seeds); seed_p++) {
    size_t num = *seed_p;

    for(MapType map_type = FIRST_MAP; map_type <= LAST_MAP; map_type++) {
      TRY(map_number(num, &almanac->maps[map_type], &num));
    }

    lowest = (num < lowest) ? num : lowest;
  }

  CHECK(lowest != SIZE_MAX);
  *out = lowest;

  return OK;
}