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

  DAR_DArray input_seq = {0};
  TRY(DAR_create(&input_seq, sizeof(TransitionType)));
  TRY(parse_input_sequence(DAR_to_span(DAR_first(&lines)), &input_seq));

  StateMachine machine = {0};
  TRY(parse_state_machine(SPN_subspan(DAR_to_span(&lines), 2, lines.size - 2), &machine));

  size_t number_of_steps = 0;
  TRY(get_number_of_steps_for_input_on_state_machine(&machine, DAR_to_span(&input_seq), &number_of_steps));

  TRY(destroy_state_machine(&machine));
  TRY(DAR_destroy(&input_seq));
  TRY(destroy_lines(&lines));

  return LOG_STAT(STAT_OK, "number_of_steps: %zu", number_of_steps);
}
