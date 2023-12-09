#ifndef lib_h
#define lib_h

#include <cfac/stat.h>
#include <cfac/span.h>

typedef struct Race {
  size_t time;
  size_t distance;
} Race;

STAT_Val get_record_beating_input_product(SPN_Span races, size_t * out);

#endif