#ifndef lib_h
#define lib_h

#include <cfac/stat.h>
#include <cfac/darray.h>
#include <cfac/span.h>

STAT_Val get_numbers_from_schematic(SPN_Span schematic, DAR_DArray * numbers);

#endif