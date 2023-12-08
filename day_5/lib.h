#ifndef lib_h
#define lib_h

#include <cfac/darray.h>
#include <cfac/stat.h>

typedef struct MapRange {
  size_t dst_start;
  size_t src_start;
  size_t length;
} MapRange;

typedef enum MapType {
  SEED_TO_SOIL = 0,
  SOIL_TO_FERTILIZER,
  FERTILIZER_TO_WATER,
  WATER_TO_LIGHT,
  LIGHT_TO_TEMPERATURE,
  TEMPERATURE_TO_HUMIDITY,
  HUMIDITY_TO_LOCATION,
  NUM_MAP_TYPES,
  FIRST_MAP = 0,
  LAST_MAP  = NUM_MAP_TYPES - 1,
} MapType;

inline static const char * map_type_to_str(MapType type) {
  switch(type) {
  case SEED_TO_SOIL: return "seed-to-soil";
  case SOIL_TO_FERTILIZER: return "soil-to-fertilizer";
  case FERTILIZER_TO_WATER: return "fertilizer-to-water";
  case WATER_TO_LIGHT: return "water-to-light";
  case LIGHT_TO_TEMPERATURE: return "light-to-temperature";
  case TEMPERATURE_TO_HUMIDITY: return "temperature-to-humidity";
  case HUMIDITY_TO_LOCATION: return "humidity-to-location";
  default: return "UNKNOWN MAP TYPE";
  }
}

typedef struct Almanac {
  DAR_DArray seeds;
  DAR_DArray maps[NUM_MAP_TYPES];
} Almanac;

STAT_Val parse_almanac(const DAR_DArray * lines, Almanac * out);
STAT_Val destroy_almanac(Almanac * almanac);
STAT_Val find_lowest_location_number_for_part1(const Almanac * almanac, size_t * out);
STAT_Val find_lowest_location_number_for_part2(const Almanac * almanac, size_t * out);

#endif