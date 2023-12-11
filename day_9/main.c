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

STAT_Val make_spans_for_sequences(const DAR_DArray * sequences, DAR_DArray * spans) {
  CHECK(sequences != NULL);
  CHECK(DAR_is_initialized(sequences));
  CHECK(!DAR_is_empty(sequences));
  CHECK(spans != NULL);
  CHECK(DAR_is_initialized(spans));
  CHECK(DAR_is_empty(spans));

  TRY(DAR_reserve(spans, sequences->size));

  for(size_t i = 0; i < sequences->size; i++) {
    const DAR_DArray * e    = DAR_get(sequences, i);
    SPN_Span           span = DAR_to_span(e);
    TRY(DAR_push_back(spans, &span));
  }

  return OK;
}

int main(void) {
  DAR_DArray lines = {0};
  TRY(DAR_create(&lines, sizeof(DAR_DArray)));

  TRY(read_lines_from_file("input.txt", &lines));

  DAR_DArray sequences = {0};
  TRY(DAR_create(&sequences, sizeof(DAR_DArray)));
  TRY(DAR_reserve(&sequences, lines.size));

  for(const DAR_DArray * line = DAR_first(&lines); line != DAR_end(&lines); line++) {
    DAR_DArray sequence = {0};
    TRY(DAR_create(&sequence, sizeof(ssize_t)));

    TRY(parse_sequence_line(DAR_to_span(line), &sequence));
    TRY(DAR_push_back(&sequences, &sequence));
  }

  DAR_DArray sequence_spans = {0};
  TRY(DAR_create(&sequence_spans, sizeof(SPN_Span)));

  TRY(make_spans_for_sequences(&sequences, &sequence_spans));

  SPN_Span sequences_span = DAR_to_span(&sequence_spans);

  ssize_t next_value_sum = 0;
  ssize_t prev_value_sum = 0;
  TRY(get_sum_of_next_values_in_sequences(sequences_span, &next_value_sum));
  TRY(get_sum_of_prev_values_in_sequences(sequences_span, &prev_value_sum));

  TRY(DAR_destroy(&sequence_spans));

  for(DAR_DArray * sequence = DAR_first(&sequences); sequence != DAR_end(&sequences); sequence++) {
    TRY(DAR_destroy(sequence));
  }

  TRY(DAR_destroy(&sequences));
  TRY(destroy_lines(&lines));

  return LOG_STAT(STAT_OK, "next_value_sum: %zd, prev_value_sum: %zd", next_value_sum, prev_value_sum);
}
