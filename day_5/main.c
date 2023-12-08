#include <stdio.h>
#include <stdlib.h>

#include <cfac/darray.h>
#include <cfac/log.h>
#include <cfac/stat.h>

#include "common.h"
#include "lib.h"

static STAT_Val read_lines_from_file(const char * filename, DAR_DArray * lines) {
  CHECK(lines != NULL);
  CHECK(DAR_is_initialized(lines));

  FILE * file_ptr = fopen(filename, "r");
  CHECK(file_ptr != NULL);

  char *  line_raw = NULL;
  size_t  len      = 0;
  ssize_t read     = 0;

  while((read = getline(&line_raw, &len, file_ptr)) != -1) {
    DAR_DArray line = {0};
    TRY(DAR_create_from_cstr(&line, line_raw));
    TRY(DAR_push_back(lines, &line));
  }

  fclose(file_ptr);
  free(line_raw);

  return OK;
}

static STAT_Val destroy_lines(DAR_DArray * lines) {
  CHECK(lines != NULL);

  for(DAR_DArray * p = DAR_first(lines); p != DAR_end(lines); p++) { TRY(DAR_destroy(p)); }

  TRY(DAR_destroy(lines));

  return OK;
}

int main(void) {
  DAR_DArray lines = {0};
  TRY(DAR_create(&lines, sizeof(DAR_DArray)));

  TRY(read_lines_from_file("input.txt", &lines));

  Almanac almanac                   = {0};
  size_t  lowest_location_for_part1 = 0;
  size_t  lowest_location_for_part2 = 0;
  TRY(parse_almanac(&lines, &almanac));
  TRY(find_lowest_location_number_for_part1(&almanac, &lowest_location_for_part1));
  TRY(find_lowest_location_number_for_part2(&almanac, &lowest_location_for_part2));

  TRY(destroy_almanac(&almanac));
  TRY(destroy_lines(&lines));

  return LOG_STAT(STAT_OK,
                  "lowest_location_for_part1: %zu, lowest_location_for_part_2: %zu",
                  lowest_location_for_part1,
                  lowest_location_for_part2);
}
