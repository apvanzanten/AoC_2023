#ifndef lib_h
#define lib_h

#include <cfac/darray.h>
#include <cfac/span.h>
#include <cfac/stat.h>

STAT_Val get_card_score(SPN_Span card_line, int * score);

STAT_Val get_card_win_count(SPN_Span card_line, size_t * win_count);

STAT_Val get_total_number_of_cards(const DAR_DArray * lines, size_t * num_of_cards);

#endif