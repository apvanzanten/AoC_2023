#ifndef lib_h
#define lib_h

#include <cfac/darray.h>
#include <cfac/stat.h>

typedef struct Record {
  uint64_t   operational_bits;
  uint64_t   damaged_bits;
  uint64_t   unknown_bits;
  DAR_DArray groups; // contains size_t
} Record;

void print_binary(size_t n, size_t num_bits_to_print);

STAT_Val parse_records(const DAR_DArray * lines, DAR_DArray * records /*contains Record*/);

STAT_Val get_num_possibilities_for_record(const Record * record, size_t * num_possibilities);
STAT_Val get_num_possibilities_for_all_records(const DAR_DArray * records, size_t * num_possibilities);

STAT_Val print_records(const DAR_DArray * records);

STAT_Val destroy_records(DAR_DArray * records);

#endif