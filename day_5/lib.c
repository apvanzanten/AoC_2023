#include <cfac/darray.h>
#include <cfac/list.h>
#include <cfac/log.h>

#include <stdio.h>
#include <stdlib.h>

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

static int compare_map_ranges_by_src_start(const void * a, const void * b) {
  const MapRange * range_a = a;
  const MapRange * range_b = b;
  return (range_a->src_start > range_b->src_start) ? 1 : (range_a->src_start < range_b->src_start) ? -1 : 0;
}

static STAT_Val sort_map_ranges_by_src_start(DAR_DArray * ranges) {
  CHECK(ranges != NULL);
  CHECK(ranges->element_size == sizeof(MapRange));

  qsort(ranges->data, ranges->size, ranges->element_size, compare_map_ranges_by_src_start);

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
    TRY(sort_map_ranges_by_src_start(&almanac->maps[map_type]));
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

STAT_Val find_lowest_location_number_for_part1(const Almanac * almanac, size_t * out) {
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

typedef struct Range {
  size_t start;
  size_t length;
} Range;

static size_t max_sz(size_t a, size_t b) { return (a > b) ? a : b; }
static size_t min_sz(size_t a, size_t b) { return (a < b) ? a : b; }

static bool is_ranges_overlapping(Range a, Range b) {
  size_t a_first = a.start;
  size_t b_first = b.start;
  size_t a_last  = (a.start + a.length - 1);
  size_t b_last  = (b.start + b.length - 1);

  return (a_first >= b_first && a_first <= b_last) || (a_last >= b_first && a_last <= b_last);
}

static Range get_overlap(Range a, Range b) {
  size_t a_first = a.start;
  size_t b_first = b.start;
  size_t a_last  = (a.start + a.length - 1);
  size_t b_last  = (b.start + b.length - 1);

  const size_t overlap_first = max_sz(a_first, b_first);
  const size_t overlap_last  = min_sz(a_last, b_last);

  return (Range){.start = overlap_first, .length = (overlap_last - overlap_first) + 1};
}

static bool is_ranges_directly_adjacent(Range a, Range b) {
  return (a.start == (b.start + b.length)) || (b.start == (a.start + a.length));
}

static Range add_ranges(Range a, Range b) {
  if(!(is_ranges_overlapping(a, b) || is_ranges_directly_adjacent(a, b))) {
    LOG_STAT(STAT_ERR_ARGS,
             "ranges not overlapping or adjacent: {%zu, %zu}, {%zu, %zu}",
             a.start,
             a.length,
             b.start,
             b.length);
  }

  const size_t a_end  = a.start + a.length;
  const size_t b_end  = b.start + b.length;
  const size_t start  = min_sz(a.start, b.start);
  const size_t length = max_sz(a_end, b_end) - start;

  return (Range){start, length};
}

static STAT_Val reduce_ranges(DAR_DArray * ranges) {
  CHECK(ranges != NULL);
  CHECK(ranges->size > 0);
  CHECK(ranges->element_size == sizeof(Range));

  LST_List l = {0};
  TRY(LST_create(&l, sizeof(Range)));
  TRY(LST_insert_from_array(&l, LST_end(&l), ranges->data, ranges->size, NULL));

  TRY(DAR_clear(ranges));

  LST_Node * node_a = LST_first(&l);

  while(node_a != LST_end(&l)) {
    Range range = *(Range *)LST_data(node_a);

    LST_Node * node_b = node_a->next;

    while(node_b != LST_end(&l)) {
      const Range other_range = *(Range *)LST_data(node_b);

      if(is_ranges_overlapping(range, other_range) || is_ranges_directly_adjacent(range, other_range)) {
        range = add_ranges(range, other_range);

        node_b = node_b->next;
        TRY(LST_remove(node_b->prev));
      } else {
        node_b = node_b->next;
      }
    }

    node_a = node_a->next;
  }

  for(LST_Node * n = LST_first(&l); n != LST_end(&l); n = n->next) { TRY(DAR_push_back(ranges, LST_data(n))); }

  TRY(DAR_shrink_to_fit(ranges));

  TRY(LST_destroy(&l));

  return OK;
}

static int compare_ranges_by_start(const void * a, const void * b) {
  const Range * range_a = a;
  const Range * range_b = b;
  return (range_a->start > range_b->start) ? 1 : (range_a->start < range_b->start) ? -1 : 0;
}

static STAT_Val sort_ranges_by_start(DAR_DArray * ranges) {
  CHECK(ranges != NULL);
  CHECK(ranges->element_size == sizeof(Range));

  qsort(ranges->data, ranges->size, ranges->element_size, compare_ranges_by_start);

  return OK;
}

static STAT_Val get_inverted_ranges_from_map_ranges(const DAR_DArray * map_ranges, DAR_DArray * out) {
  CHECK(map_ranges != NULL);
  CHECK(map_ranges->size > 0);
  CHECK(map_ranges->element_size == sizeof(MapRange));
  CHECK(out != NULL);
  CHECK(out->element_size == sizeof(Range));
  CHECK(DAR_is_initialized(out));

  size_t cursor = 0;
  for(const MapRange * map = DAR_first(map_ranges); map != DAR_end(map_ranges); map++) {
    if(cursor < map->src_start) {
      const Range range = {.start = cursor, .length = (map->src_start - cursor)};
      TRY(DAR_push_back(out, &range));
    }
    if(cursor < map->src_start + map->length) cursor = (map->src_start + map->length);
  }

  const Range final_range = {.start = cursor, .length = (SIZE_MAX - cursor)};
  TRY(DAR_push_back(out, &final_range));

  TRY(reduce_ranges(out));
  TRY(sort_ranges_by_start(out));

  return OK;
}

static STAT_Val map_range_to_ranges(const DAR_DArray * map_ranges,
                                    const DAR_DArray * inverse_map_ranges,
                                    Range              input,
                                    DAR_DArray *       out) {
  CHECK(map_ranges != NULL);
  CHECK(map_ranges->size > 0);
  CHECK(map_ranges->element_size == sizeof(MapRange));
  CHECK(input.length > 0);
  CHECK(out != NULL);
  CHECK(out->element_size == sizeof(Range));
  CHECK(DAR_is_initialized(out));

  // ASSUMPTION: map_ranges is sorted by range src_start

  // first define modified ranges
  for(const MapRange * map = DAR_first(map_ranges); map != DAR_end(map_ranges); map++) {
    Range         map_src_range = {map->src_start, map->length};
    const int64_t offset        = map->dst_start - map->src_start;

    if(is_ranges_overlapping(input, map_src_range)) {
      const Range overlap   = get_overlap(input, map_src_range);
      const Range dst_range = {.start = overlap.start + offset, .length = overlap.length};
      TRY(DAR_push_back(out, &dst_range));
    }
  }

  // TODO probably move this out of here and into outer loop
  // then define unmodified ranges by checking overlap with the inverse of the map ranges

  for(const Range * range = DAR_first(inverse_map_ranges); range != DAR_end(inverse_map_ranges); range++) {
    if(is_ranges_overlapping(input, *range)) {
      const Range overlap = get_overlap(input, *range);
      TRY(DAR_push_back(out, &overlap));
    }
  }

  TRY(reduce_ranges(out));
  TRY(sort_ranges_by_start(out));

  return OK;
}

static STAT_Val make_seed_ranges(const DAR_DArray * seeds, DAR_DArray * ranges) {
  CHECK(ranges != NULL);
  CHECK(DAR_is_initialized(seeds));
  CHECK(seeds->element_size == sizeof(size_t));
  CHECK(seeds->size > 0);
  CHECK((seeds->size % 2) == 0);
  CHECK(DAR_is_initialized(ranges));
  CHECK(ranges->element_size == sizeof(Range));

  TRY(DAR_reserve(ranges, (seeds->size / 2)));

  for(size_t idx = 0; (idx + 1) < seeds->size; idx += 2) {
    Range seed_range = {.start  = *(const size_t *)DAR_get(seeds, idx),
                        .length = *(const size_t *)DAR_get(seeds, idx + 1)};
    TRY(DAR_push_back(ranges, &seed_range));
  }

  TRY(sort_ranges_by_start(ranges));

  return OK;
}

STAT_Val find_lowest_location_number_for_part2(const Almanac * almanac, size_t * out) {
  CHECK(almanac != NULL);
  CHECK(out != NULL);

  DAR_DArray work_ranges        = {0};
  DAR_DArray tmp_ranges         = {0};
  DAR_DArray inverse_map_ranges = {0};
  TRY(DAR_create(&work_ranges, sizeof(Range)));
  TRY(DAR_create(&tmp_ranges, sizeof(Range)));
  TRY(DAR_create(&inverse_map_ranges, sizeof(Range)));

  TRY(make_seed_ranges(&almanac->seeds, &work_ranges));

  for(MapType map_type = FIRST_MAP; map_type <= LAST_MAP; map_type++) {
    TRY(get_inverted_ranges_from_map_ranges(&almanac->maps[map_type], &inverse_map_ranges));

    // produce new set of seed ranges based on mapping, placed in tmp_ranges
    for(const Range * range = DAR_first(&work_ranges); range != DAR_end(&work_ranges); range++) {
      TRY(map_range_to_ranges(&almanac->maps[map_type], &inverse_map_ranges, *range, &tmp_ranges));
    }

    // 'swap' work and tmp ranges, by exchanging pointers and sizes, then clear tmp_ranges
    {
      void * new_tmp_data = work_ranges.data;
      size_t new_tmp_size = work_ranges.size;
      work_ranges.data    = tmp_ranges.data;
      work_ranges.size    = tmp_ranges.size;
      tmp_ranges.data     = new_tmp_data;
      tmp_ranges.size     = new_tmp_size;
    }
    TRY(DAR_clear(&tmp_ranges));

    // finally, clear the inverted map ranges, as we're moving on to the next map
    TRY(DAR_clear(&inverse_map_ranges));

    CHECK(!DAR_is_empty(&work_ranges));
  }

  *out = ((Range *)DAR_first(&work_ranges))->start;

  TRY(DAR_destroy(&work_ranges));
  TRY(DAR_destroy(&tmp_ranges));
  TRY(DAR_destroy(&inverse_map_ranges));

  return OK;
}