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

  DAR_DArray schematic_arr = {0};
  TRY(DAR_create(&schematic_arr, sizeof(SPN_Span)));

  for(size_t i = 0; i < lines.size; i++) {
    SPN_Span line = DAR_to_span(DAR_get(&lines, i));
    TRY(DAR_push_back(&schematic_arr, &line));
  }

  DAR_DArray numbers = {0};
  TRY(DAR_create(&numbers, sizeof(int)));

  TRY(get_numbers_from_schematic(DAR_to_span(&schematic_arr), &numbers));

  int sum_of_numbers = 0;
  for(int * p = DAR_first(&numbers); p != DAR_end(&numbers); p++) { 
    sum_of_numbers += *p; 
  }

  TRY(destroy_lines(&lines));
  TRY(DAR_destroy(&numbers));
  TRY(DAR_destroy(&schematic_arr));

  return LOG_STAT(STAT_OK, "sum_of_numbers: %d", sum_of_numbers);
}
