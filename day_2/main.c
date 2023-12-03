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

  int sum_of_possible_ids = 0;
  int sum_of_powers       = 0;
  for(DAR_DArray * line = DAR_first(&lines); line != DAR_end(&lines); line++) {
    GameResult res = {0};
    TRY(get_game_result_from_line(DAR_to_span(line), &res));
    if(res.min_color_occurrences[RED] <= 12 && res.min_color_occurrences[GREEN] <= 13 &&
       res.min_color_occurrences[BLUE] <= 14) {
      sum_of_possible_ids += res.game_id;
    }
    sum_of_powers +=
        (res.min_color_occurrences[RED] * res.min_color_occurrences[GREEN] * res.min_color_occurrences[BLUE]);
  }

  TRY(destroy_lines(&lines));

  return LOG_STAT(STAT_OK, "sum_of_possible_ids: %d, sum_of_powers: %d", sum_of_possible_ids, sum_of_powers);
}
