#include <cfac/log.h>

#include "common.h"
#include "lib.h"

#include <stdio.h>

STAT_Val get_delta_sequence(SPN_Span sequence, SPN_MutSpan deltas) {
  CHECK(!SPN_is_empty(sequence));
  CHECK(sequence.element_size == sizeof(ssize_t));
  CHECK(!SPN_is_empty(SPN_mut_to_const(deltas)));
  CHECK(deltas.element_size == sizeof(ssize_t));
  CHECK(deltas.len == sequence.len - 1);

  const ssize_t * seq       = sequence.begin;
  ssize_t *       delta_seq = deltas.begin;

  for(size_t i = 0; i < deltas.len; i++) { delta_seq[i] = seq[i + 1] - seq[i]; }

  return OK;
}

static bool is_sequence_all_zeroes(SPN_Span sequence) {
  if(sequence.element_size != sizeof(ssize_t)) return false;
  if(SPN_is_empty(sequence)) return false;

  for(const ssize_t * p = SPN_first(sequence); p != SPN_end(sequence); p++) {
    if(*p != 0) return false;
  }

  return true;
}

STAT_Val generate_histories_for_sequence(SPN_Span sequence, DAR_DArray * histories /* darray of darrays of ssize_t*/) {
  CHECK(!SPN_is_empty(sequence));
  CHECK(sequence.element_size == sizeof(ssize_t));
  CHECK(histories != NULL);
  CHECK(histories->element_size == sizeof(DAR_DArray));
  CHECK(DAR_is_empty(histories));

  SPN_Span work_seq = sequence;
  while((work_seq.len > 1) && !is_sequence_all_zeroes(work_seq)) {
    DAR_DArray history = {0};
    TRY(DAR_create(&history, sizeof(ssize_t)));
    TRY(DAR_resize_zeroed(&history, work_seq.len - 1));

    TRY(get_delta_sequence(work_seq, DAR_to_mut_span(&history)));

    TRY(DAR_push_back(histories, &history));

    work_seq = DAR_to_span(DAR_last(histories));
  }

  return OK;
}

STAT_Val get_next_value_in_sequence(SPN_Span sequence, ssize_t * next_value) {
  CHECK(!SPN_is_empty(sequence));
  CHECK(sequence.element_size == sizeof(ssize_t));
  CHECK(next_value != NULL);

  DAR_DArray histories = {0};
  TRY(DAR_create(&histories, sizeof(DAR_DArray)));

  TRY(generate_histories_for_sequence(sequence, &histories));

  ssize_t val = 0;
  for(DAR_DArray * hist = DAR_first(&histories); hist != DAR_end(&histories); hist++) {
    val += *(ssize_t *)DAR_last(hist);
  }

  val += *(const ssize_t *)SPN_last(sequence);

  *next_value = val;

  TRY(destroy_histories(&histories));

  return OK;
}

STAT_Val get_prev_value_in_sequence(SPN_Span sequence, ssize_t * prev_value) {
  CHECK(!SPN_is_empty(sequence));
  CHECK(sequence.element_size == sizeof(ssize_t));
  CHECK(prev_value != NULL);

  DAR_DArray histories = {0};
  TRY(DAR_create(&histories, sizeof(DAR_DArray)));

  TRY(generate_histories_for_sequence(sequence, &histories));

  ssize_t val = 0;

  for(ssize_t i = histories.size - 1; i >= 0; i--) {
    val = *(ssize_t *)DAR_first((DAR_DArray *)DAR_get(&histories, i)) - val;
  }

  val = *(ssize_t *)SPN_first(sequence) - val;

  *prev_value = val;

  TRY(destroy_histories(&histories));

  return OK;
}

STAT_Val get_sum_of_next_values_in_sequences(SPN_Span sequences /* SPN_Span of SPN_Span of ssize_t*/, ssize_t * sum) {
  CHECK(!SPN_is_empty(sequences));
  CHECK(sequences.element_size == sizeof(SPN_Span));
  CHECK(sum != NULL);

  *sum = 0;
  for(const SPN_Span * seq = SPN_first(sequences); seq != SPN_end(sequences); seq++) {
    ssize_t val = 0;
    TRY(get_next_value_in_sequence(*seq, &val));
    (*sum) += val;
  }

  return OK;
}

STAT_Val get_sum_of_prev_values_in_sequences(SPN_Span sequences /* SPN_Span of SPN_Span of ssize_t*/, ssize_t * sum) {
  CHECK(!SPN_is_empty(sequences));
  CHECK(sequences.element_size == sizeof(SPN_Span));
  CHECK(sum != NULL);

  *sum = 0;
  for(const SPN_Span * seq = SPN_first(sequences); seq != SPN_end(sequences); seq++) {
    ssize_t val = 0;
    TRY(get_prev_value_in_sequence(*seq, &val));
    (*sum) += val;
  }

  return OK;
}

STAT_Val parse_sequence_line(SPN_Span line, DAR_DArray * sequence) {
  CHECK(!SPN_is_empty(line));
  CHECK(sequence != NULL);
  CHECK(DAR_is_initialized(sequence));
  CHECK(DAR_is_empty(sequence));

  const char delim = ' ';

  SPN_Span remaining = line;
  while(remaining.len > 0) {
    ssize_t v = 0;
    CHECK(sscanf(remaining.begin, "%zd", &v) == 1);

    TRY(DAR_push_back(sequence, &v));

    size_t   next_delim_idx = 0;
    STAT_Val find_delim_st  = SPN_find(remaining, &delim, &next_delim_idx);
    CHECK(STAT_is_OK(find_delim_st));
    if(find_delim_st == STAT_OK) {
      remaining = SPN_subspan(remaining, next_delim_idx + 1, (remaining.len - next_delim_idx));
    } else if(find_delim_st == STAT_OK_NOT_FOUND) {
      break;
    }
  }

  return OK;
}

STAT_Val destroy_histories(DAR_DArray * histories /* darray of darrays of ssize_t*/) {
  CHECK(histories != NULL);

  for(DAR_DArray * hist = DAR_first(histories); hist != DAR_end(histories); hist++) { TRY(DAR_destroy(hist)); }

  TRY(DAR_destroy(histories));

  return OK;
}