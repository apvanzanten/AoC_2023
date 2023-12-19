#ifndef LIB_H
#define LIB_H

#include <cfac/darray.h>
#include <cfac/span.h>
#include <cfac/stat.h>

typedef struct Pattern {
  DAR_DArray rows; // will contain size_t
  size_t     width;
} Pattern;

STAT_Val parse_pattern(SPN_Span lines, Pattern * out);

STAT_Val transpose_pattern(const Pattern * in, Pattern * out);

STAT_Val find_mirror(const Pattern * in, bool * has_mirror, size_t * mirror_position);

STAT_Val destroy_pattern(Pattern * pattern);

STAT_Val print_pattern(const Pattern * pattern);

#endif