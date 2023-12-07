#ifndef lib_h
#define lib_h

#include <cfac/stat.h>
#include <cfac/darray.h>

typedef struct MapRange {
  size_t src_start;
  size_t dst_start;
  size_t length;
} MapRange;

typedef struct Almanac {
  DAR_DArray seeds;
  DAR_DArray seed_to_soil_map_ranges;
  DAR_DArray soil_to_fertilizer_map_ranges;
  DAR_DArray fertilizer_to_water_map_ranges;
  DAR_DArray water_to_light_map_ranges;
  DAR_DArray light_to_temperature_map_ranges;
  DAR_DArray temperature_to_humidity_map_ranges;
  DAR_DArray humidity_to_location_map_ranges;
} Almanac;

STAT_Val parse_almanac(const DAR_DArray * lines, Almanac * out);
STAT_Val destroy_almanac(Almanac * almanac);

#endif