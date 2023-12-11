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

  size_t   max_dist     = 0;
  Position max_dist_pos = {0};

  PipeSketch sketch = {0};
  TRY(parse_sketch(&lines, &sketch));
  TRY(calculate_distances_from_start(&sketch));
  TRY(get_max_distance_from_start(&sketch, &max_dist, &max_dist_pos));

  TRY(destroy_sketch(&sketch));
  TRY(destroy_lines(&lines));

  return LOG_STAT(STAT_OK, "max_dist: %zu, max_dist_pos: (%zu,%zu)", max_dist, max_dist_pos.x, max_dist_pos.y);
}
