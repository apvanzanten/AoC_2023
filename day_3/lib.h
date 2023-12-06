#ifndef lib_h
#define lib_h

#include <cfac/darray.h>
#include <cfac/span.h>
#include <cfac/stat.h>

STAT_Val get_numbers_from_schematic(SPN_Span schematic, DAR_DArray * numbers);
STAT_Val get_gear_ratios_from_schematic(SPN_Span schematic, DAR_DArray * ratios);

#endif