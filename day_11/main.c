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

  Universe   universe               = {0};
  DAR_DArray galaxy_positions       = {0};
  DAR_DArray distances              = {0};
  size_t     sum_of_distances_part1 = 0;
  size_t     sum_of_distances_part2 = 0;

  TRY(DAR_create(&galaxy_positions, sizeof(Position)));
  TRY(DAR_create(&distances, sizeof(size_t)));

  TRY(parse_universe(&lines, &universe));

  TRY(calculate_galaxy_distances(&universe, &galaxy_positions, &distances));
  TRY(sum_galaxy_distances(&universe, &distances, galaxy_positions.size, &sum_of_distances_part1));

  TRY(DAR_clear(&galaxy_positions));
  TRY(DAR_clear(&distances));

  TRY(increase_space_density_for_gaps(&universe, 1000000));

  TRY(calculate_galaxy_distances(&universe, &galaxy_positions, &distances));
  TRY(sum_galaxy_distances(&universe, &distances, galaxy_positions.size, &sum_of_distances_part2));

  TRY(DAR_destroy(&galaxy_positions));
  TRY(DAR_destroy(&distances));
  TRY(destroy_universe(&universe));
  TRY(destroy_lines(&lines));

  return LOG_STAT(STAT_OK,
                  "sum_of_distances_part1: %zu, sum_of_distances_part2: %zu",
                  sum_of_distances_part1,
                  sum_of_distances_part2);
}
