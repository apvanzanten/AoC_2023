#ifndef lib_h
#define lib_h

#include <cfac/darray.h>
#include <cfac/span.h>
#include <cfac/stat.h>

#include <sys/types.h>

STAT_Val get_delta_sequence(SPN_Span sequence, SPN_MutSpan deltas);

STAT_Val generate_histories_for_sequence(SPN_Span sequence, DAR_DArray * histories /* darray of darrays of ssize_t*/);

STAT_Val get_next_value_in_sequence(SPN_Span sequence, ssize_t * next_value);

STAT_Val get_prev_value_in_sequence(SPN_Span sequence, ssize_t * prev_value);

STAT_Val get_sum_of_next_values_in_sequences(SPN_Span sequences /* SPN_Span of SPN_Span of ssize_t*/, ssize_t * sum);
STAT_Val get_sum_of_prev_values_in_sequences(SPN_Span sequences /* SPN_Span of SPN_Span of ssize_t*/, ssize_t * sum);

STAT_Val parse_sequence_line(SPN_Span line, DAR_DArray * sequence);

STAT_Val destroy_histories(DAR_DArray * histories /* darray of darrays of ssize_t*/);

#endif