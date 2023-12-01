#include <cfac/log.h>

#include "lib.h"

#include <cfac/darray.h>

#include <math.h>

#include "common.h"

#define OK STAT_OK

static const char * g_text_digits[] = { "zero","one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };


static STAT_Val get_digit(SPN_Span line_remainder, int * o_digit) {
  CHECK(line_remainder.len > 0);
  CHECK(o_digit != NULL);

  const char first = *(const char*)SPN_first(line_remainder);
  if(first >= '0' && first <= '9') {
    *o_digit = first - '0';
    return OK;
  }

  for(size_t i = 0; i < sizeof(g_text_digits) / sizeof(g_text_digits[0]); i++) {
    size_t idx = 0;

    const STAT_Val find_res = SPN_find_subspan(line_remainder, SPN_from_cstr(g_text_digits[i]), &idx);
    if(find_res == STAT_OK && idx == 0) {
      *o_digit = (int)i;
      return OK;
    }
  }

  return STAT_OK_NOT_FOUND;
}

static STAT_Val get_digits(SPN_Span line, DAR_DArray * digits) {
  if(digits == NULL) return LOG_STAT(STAT_ERR_ARGS, "digits is NULL");
  if(!DAR_is_initialized(digits)) return LOG_STAT(STAT_ERR_ARGS, "digits is uninitialized");

  for(size_t i = 0; i < line.len; i++) {
    int digit = 0;
    const STAT_Val find_res = get_digit(SPN_subspan(line, i, line.len - i), &digit);
    if(find_res == STAT_OK){
      TRY(DAR_push_back(digits, &digit));
    }
  }

  return OK;
}

STAT_Val get_calibration_value(SPN_Span line, int * value) {
  CHECK(value != NULL);
  CHECK(line.len >= 2);

  DAR_DArray digits = {0};
  TRY(DAR_create(&digits, sizeof(int)));

  TRY(get_digits(line, &digits));

  CHECK(digits.size >= 1);

  *value = (*(int*)DAR_first(&digits)) * 10 + (*(int*)DAR_last(&digits));

  TRY(DAR_destroy(&digits));

  return STAT_OK;
}