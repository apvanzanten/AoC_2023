#ifndef COMMON_H
#define COMMON_H

#include <cfac/log.h>
#include <cfac/stat.h>

#include <stdbool.h>

#define OK STAT_OK

#define CHECK(x)                                                                                                       \
  do {                                                                                                                 \
    if(!(x)) return LOG_STAT(STAT_ERR_ASSERTION, "failed assertion: %s", #x);                                          \
  } while(false)

#define TRY(x)                                                                                                         \
  do {                                                                                                                 \
    STAT_Val st = (x);                                                                                                 \
    if(!STAT_is_OK(st)) return LOG_STAT(st, "failed: %s", #x);                                                         \
  } while(false)

static inline uint8_t popcount(__uint128_t n) {
  uint8_t c = 0;

  while(n != 0) {
    n >>= 1;
    c++;
  }

  return c;
}

#endif