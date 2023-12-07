#include <cfac/log.h>
#include <cfac/darray.h>

#include <stdio.h>

#include "lib.h"
#include "common.h"

static STAT_Val init_almanac(Almanac * almanac) {
  CHECK(almanac != NULL);

  TRY(DAR_create(&almanac->seeds, sizeof(size_t)));
  TRY(DAR_create(&almanac->seed_to_soil_map_ranges, sizeof(MapRange)));
  TRY(DAR_create(&almanac->soil_to_fertilizer_map_ranges, sizeof(MapRange)));
  TRY(DAR_create(&almanac->fertilizer_to_water_map_ranges, sizeof(MapRange)));
  TRY(DAR_create(&almanac->water_to_light_map_ranges, sizeof(MapRange)));
  TRY(DAR_create(&almanac->light_to_temperature_map_ranges, sizeof(MapRange)));
  TRY(DAR_create(&almanac->temperature_to_humidity_map_ranges, sizeof(MapRange)));
  TRY(DAR_create(&almanac->humidity_to_location_map_ranges, sizeof(MapRange)));

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

  SPN_Span remaining = DAR_to_span(line);
  const char delim = ' '; 
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


STAT_Val parse_almanac(const DAR_DArray * lines, Almanac * out) {
  CHECK(lines != NULL);
  CHECK(lines->element_size == sizeof(DAR_DArray));
  CHECK(DAR_is_initialized(lines));
  CHECK(!DAR_is_empty(lines));
  CHECK(out != NULL);

  TRY(init_almanac(out));
  TRY(parse_seeds(DAR_first(lines), &out->seeds));

  return OK;
}

STAT_Val destroy_almanac(Almanac * almanac) {
  CHECK(almanac != NULL);

  TRY(DAR_destroy(&almanac->seeds));
  TRY(DAR_destroy(&almanac->seed_to_soil_map_ranges));
  TRY(DAR_destroy(&almanac->soil_to_fertilizer_map_ranges));
  TRY(DAR_destroy(&almanac->fertilizer_to_water_map_ranges));
  TRY(DAR_destroy(&almanac->water_to_light_map_ranges));
  TRY(DAR_destroy(&almanac->light_to_temperature_map_ranges));
  TRY(DAR_destroy(&almanac->temperature_to_humidity_map_ranges));
  TRY(DAR_destroy(&almanac->humidity_to_location_map_ranges));

  return OK;
}