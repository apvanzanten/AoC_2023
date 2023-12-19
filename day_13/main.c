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

  SPN_Span lines_span = DAR_to_span(&lines);

  size_t end_cursor   = 0;
  size_t start_cursor = 0;

  size_t count = 0;

  while((start_cursor < lines_span.len) && (end_cursor < lines_span.len)) {
    for(end_cursor = start_cursor; end_cursor < lines_span.len; end_cursor++) {
      const DAR_DArray * line = SPN_get(lines_span, end_cursor);

      if(line->size == 0) break;

      const char * first_char = DAR_first(line);
      if(!((*first_char == '#') || *first_char == '.')) break;
    }

    SPN_Span pattern_span       = SPN_subspan(lines_span, start_cursor, (end_cursor - start_cursor));
    Pattern  pattern            = {0};
    Pattern  pattern_transposed = {0};
    TRY(parse_pattern(pattern_span, &pattern));
    TRY(transpose_pattern(&pattern, &pattern_transposed));

    size_t mirror_position = 0;
    bool   has_mirror      = false;
    TRY(find_mirror(&pattern, &has_mirror, &mirror_position));

    if(has_mirror) count += (100 * mirror_position);

    mirror_position = 0;
    has_mirror      = false;
    TRY(find_mirror(&pattern_transposed, &has_mirror, &mirror_position));

    if(has_mirror) count += mirror_position;

    TRY(destroy_pattern(&pattern));
    TRY(destroy_pattern(&pattern_transposed));

    start_cursor = end_cursor + 1;
  }

  TRY(destroy_lines(&lines));

  return LOG_STAT(STAT_OK, "count: %zu", count);
}
